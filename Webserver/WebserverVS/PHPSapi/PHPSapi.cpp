#include <php.h>
#include <php_main.h>
#include <php_globals.h>
#include <php_variables.h>
#include <SAPI.h>
#include <zend.h>
#include <zend_hash.h>
#include <zend_modules.h>
#include <zend_interfaces.h>
#include <zend_extensions.h>
#undef inline
#define _CTIME_
#include "PHPSapi.h"
#ifndef ZTS
#include <mutex>
#endif
#include <algorithm>
#include <sstream>
#include <string>
#include <experimental/filesystem>
#include <vector>
#include <unordered_map>
#include <functional>
#include <iostream>
#undef min

using namespace Net::Http;
using namespace std::experimental::filesystem;

#ifndef ZTS
std::mutex phpsync;
#endif

struct PHPClientData
{
	std::shared_ptr<Net::Http::V2::Session> session;
	std::shared_ptr<Net::Http::V2::Stream> stream;
	std::shared_ptr<Net::Http::Request> request;
	Response response;
	std::unordered_map<std::string, std::string> env;
};

static std::unordered_map<std::string, std::string> translate = {
	{ ":authority","HTTP_HOST" },
	{ "accept","HTTP_ACCEPT" },
	{ "accept-charset", "HTTP_ACCEPT_CHARSET" },
	{ "accept-encoding","HTTP_ACCEPT_ENCODING" },
	{ "accept-language","HTTP_ACCEPT_LANGUAGE" },
	{ "referer", "HTTP_REFERER" },
	{ "user-agent","HTTP_USER_AGENT" }
};

static int php_phprun_startup(sapi_module_struct *sapi_module)
{
	return php_module_startup(sapi_module, NULL, 0);
}

static size_t sapi_phprun_ub_write(const char *str, size_t str_length)
{
	if (!str_length) {
		return 0;
	}
	auto &client = *(PHPClientData*)SG(server_context);
	client.session->SendData(client.stream, (uint8_t*)str, str_length, false);
	return str_length;
}

static void sapi_phprun_flush(void *server_context)
{

}

static int sapi_phprun_send_headers(sapi_headers_struct *sapi_headers)
{
	auto &client = *(PHPClientData*)SG(server_context);
	auto &headerlist = client.response.headerlist;
	client.response.status = sapi_headers->http_response_code == 0 ? 200 : sapi_headers->http_response_code;
	//headerlist.push_back({ ":status", sapi_headers->http_response_code == 0 ? "200" : std::to_string(sapi_headers->http_response_code) });
	{
		sapi_header_struct * sapi_header = (sapi_header_struct *)zend_llist_get_first(&sapi_headers->headers);
		for (size_t i = 0; i < sapi_headers->headers.count; i++)
		{
			auto res = std::find(sapi_header->header, sapi_header->header + sapi_header->header_len, ':');
			if (res != (sapi_header->header + sapi_header->header_len))
			{
				int l = *(res + 1) == ' ' ? 2 : 1;
				std::string key(sapi_header->header, res);
				std::transform(key.begin(), key.end(), key.begin(), tolower);
				headerlist.insert({ key, std::string(res + l, sapi_header->header + sapi_header->header_len) });
				sapi_header = (sapi_header_struct *)zend_llist_get_next(&sapi_headers->headers);
			}
		}
	}
	client.session->SendResponse(client.stream, client.response, false);
	return SAPI_HEADER_SENT_SUCCESSFULLY;
}

static size_t sapi_phprun_read_post(char *buf, size_t count_bytes)
{
	auto &client = *(PHPClientData*)SG(server_context);
	return 0;
}

static char* sapi_phprun_read_cookies(void)
{
	auto &client = *(PHPClientData*)SG(server_context);
	auto res = client.request->headerlist.lower_bound("cookie");
	if (res != client.request->headerlist.end())
	{
		return (char*)res->second.data();
	}
	return NULL;
}


static void sapi_phprun_register_variables(zval *track_vars_array)
{
	auto &client = *(PHPClientData*)SG(server_context);
	for (auto &variable : client.env)
	{
		size_t value_len;
		char *key = (char*)variable.first.data(), *value = (char*)variable.second.data();
		if (sapi_module.input_filter(PARSE_SERVER, key, &value, variable.second.length(), &value_len)) {
			php_register_variable_safe(key, value, value_len, track_vars_array);
		}
	}
}

static void sapi_phprun_log_message(char *message
#ifdef _WIN32
	, int syslog_type_int
#endif
)
{
	std::cout << "PHP: '" << message << "'\n";
}

static sapi_module_struct phprun_sapi_module = {
	"phprun",						/* name */
	"PHP Runner",					/* pretty name */

	php_phprun_startup,				/* startup */
	php_module_shutdown_wrapper,	/* shutdown */

	NULL,							/* activate */
	NULL,							/* deactivate */

	sapi_phprun_ub_write,		    /* unbuffered write */
	sapi_phprun_flush,				/* flush */
	NULL,							/* get uid */
	NULL,							/* getenv */

	php_error,						/* error handler */

	NULL,							/* header handler */
	sapi_phprun_send_headers,		/* send headers handler */
	NULL,							/* send header handler */

	sapi_phprun_read_post,			/* read POST data */
	sapi_phprun_read_cookies,		/* read Cookies */

	sapi_phprun_register_variables,	/* register server variables */
	sapi_phprun_log_message,		/* Log message */
	NULL,							/* Get request time */
	NULL,							/* Child terminate */

	STANDARD_SAPI_MODULE_PROPERTIES
};

void PHPSapi::init()
{
#ifdef ZTS
	tsrm_startup(4, 4, 0, NULL);
#else
	sapi_startup(&phprun_sapi_module);
	if (phprun_sapi_module.startup(&phprun_sapi_module) == FAILURE) {
		std::cout << "module nicht gestartet" << "\n";
		return;
	}
	php_init_config();
	// php_request_startup();
#endif
}

void PHPSapi::deinit()
{
	phprun_sapi_module.shutdown(&phprun_sapi_module);
#ifdef ZTS
	tsrm_shutdown();
#endif
}

#ifdef ZTS
int resources = 0;
#endif

void PHPSapi::requesthandler(std::shared_ptr<Net::Http::V2::Session> session, std::shared_ptr<Net::Http::V2::Stream> stream, std::shared_ptr<Net::Http::Request> request)
{
	path phpfile = L"D:\\Web" / request->path;
#ifdef ZTS
	if (tsrm_get_ls_cache() == nullptr)
	{
		int res = resources++;
		ts_resource(res);
		if (res == 0)
		{
			sapi_startup(&phprun_sapi_module);
			if (phprun_sapi_module.startup(&phprun_sapi_module) == FAILURE) {
				std::cout << "module nicht gestartet" << "\n";
				return;
			}
			php_init_config();
		}
		
		//ts_free_id(resources - 1);

	}
#endif
	{
		zend_file_handle file_handle;
		PHPClientData info = { session, stream, request };
		std::string phpfilestring = phpfile.u8string();
		file_handle.type = ZEND_HANDLE_FILENAME;
		file_handle.filename = (char *)phpfilestring.c_str();
		file_handle.free_filename = 0;
		file_handle.opened_path = NULL;

		SG(request_info).query_string = (char*)(info.env["QUERY_STRING"] = request->query).data();
		SG(request_info).proto_num = 200;
		SG(request_info).path_translated = (char *)phpfilestring.c_str();
		SG(request_info).content_length = 0;
		SG(request_info).request_method = (char *)(info.env["REQUEST_METHOD"] = request->method).data();
		SG(request_info).content_length = request->contentlength;
		SG(request_info).content_type = request->contenttype.data();
		info.env["DOCUMENT_ROOT"] = "D:\\Web";
		info.env["HTTPS"] = "1";
		info.env["SERVER_PROTOCOL"] = "h2";
		info.env["HTTP_CONNECTION"] = "Keep-Alive";
		info.env["SCRIPT_FILENAME"] = phpfilestring;
		info.env["PHP_SELF"] = request->path;
		info.env["SCRIPT_NAME"] = request->path;
		info.env["REQUEST_URI"] = request->uri;
		for (auto &entry : request->headerlist)
		{
			auto key = translate.lower_bound(entry.first);
			if (key != translate.end())
			{
				info.env[key->second] = entry.second;
			}
		}
#ifndef ZTS
		phpsync.lock();
#endif
		SG(server_context) = &info;
		php_request_startup();
		php_execute_script(&file_handle);
		php_request_shutdown(0);
		SG(server_context) = nullptr;
#ifndef ZTS
		phpsync.unlock();
#endif
	}
	session->SendData(stream, 0, 0, true);
}