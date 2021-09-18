#pragma once
#include "../../Net/Http/V2/Session.h"
#include "../../Net/Http/V2/Stream.h"
#include "../../Net/Http/Request.h"
#include <vector>
namespace PHPSapi
{
	void init();
	void deinit();
	void requesthandler(std::shared_ptr<Net::Http::V2::Session> session, std::shared_ptr<Net::Http::V2::Stream> stream, std::shared_ptr<Net::Http::Request> request);
}