#include <jnivm/class.h>
#include <jnivm/method.h>

using namespace jnivm;

template<class T> jvalue toJValue(T val) {
	jvalue ret;
	static_assert(sizeof(T) <= sizeof(jvalue), "jvalue cannot hold the specified type!");
	memset(&ret, 0, sizeof(ret));
	memcpy(&ret, &val, sizeof(val));
	return ret;
}

jvalue Method::jinvoke(jnivm::ENV &env, jclass cl, ...) {
	if(signature.empty()) {
		throw std::runtime_error("jni signature is empty");
	}
	va_list l;
	va_start(l, cl);
	jvalue ret;
	auto type = signature[signature.find_last_of(')') + 1];
	switch (type) {
	case 'V':
		env.GetJNIEnv()->functions->CallStaticVoidMethodV(env.GetJNIEnv(), cl, (jmethodID)this, l);
		ret = {};
		break;
	case 'Z':
		ret = toJValue(env.GetJNIEnv()->functions->CallStaticBooleanMethodV(env.GetJNIEnv(), cl, (jmethodID)this, l));
		break;
	case 'B':
		ret = toJValue(env.GetJNIEnv()->functions->CallStaticByteMethodV(env.GetJNIEnv(), cl, (jmethodID)this, l));
		break;
	case 'S':
		ret = toJValue(env.GetJNIEnv()->functions->CallStaticShortMethodV(env.GetJNIEnv(), cl, (jmethodID)this, l));
		break;
	case 'I':
		ret = toJValue(env.GetJNIEnv()->functions->CallStaticIntMethodV(env.GetJNIEnv(), cl, (jmethodID)this, l));
		break;
	case 'J':
		ret = toJValue(env.GetJNIEnv()->functions->CallStaticLongMethodV(env.GetJNIEnv(), cl, (jmethodID)this, l));
		break;
	case 'F':
		ret = toJValue(env.GetJNIEnv()->functions->CallStaticFloatMethodV(env.GetJNIEnv(), cl, (jmethodID)this, l));
		break;
	case 'D':
		ret = toJValue(env.GetJNIEnv()->functions->CallStaticDoubleMethodV(env.GetJNIEnv(), cl, (jmethodID)this, l));
		break;
	case '[':
	case 'L':
		ret = toJValue(env.GetJNIEnv()->functions->CallStaticObjectMethodV(env.GetJNIEnv(), cl, (jmethodID)this, l));
		break;
	default:
		va_end(l);
		throw std::runtime_error("Unsupported signature");
	}
	va_end(l);
	return ret;
}

jvalue Method::jinvoke(jnivm::ENV &env, jobject obj, ...) {
	if(signature.empty()) {
		throw std::runtime_error("jni signature is empty");
	}
	va_list l;
	va_start(l, obj);
	jvalue ret;
	auto type = signature[signature.find_last_of(')') + 1];
	switch (type) {
	case 'V':
		env.GetJNIEnv()->functions->CallVoidMethodV(env.GetJNIEnv(), obj, (jmethodID)this, l);
		ret = {};
		break;
	case 'Z':
		ret = toJValue(env.GetJNIEnv()->functions->CallBooleanMethodV(env.GetJNIEnv(), obj, (jmethodID)this, l));
		break;
	case 'B':
		ret = toJValue(env.GetJNIEnv()->functions->CallByteMethodV(env.GetJNIEnv(), obj, (jmethodID)this, l));
		break;
	case 'S':
		ret = toJValue(env.GetJNIEnv()->functions->CallShortMethodV(env.GetJNIEnv(), obj, (jmethodID)this, l));
		break;
	case 'I':
		ret = toJValue(env.GetJNIEnv()->functions->CallIntMethodV(env.GetJNIEnv(), obj, (jmethodID)this, l));
		break;
	case 'J':
		ret = toJValue(env.GetJNIEnv()->functions->CallLongMethodV(env.GetJNIEnv(), obj, (jmethodID)this, l));
		break;
	case 'F':
		ret = toJValue(env.GetJNIEnv()->functions->CallFloatMethodV(env.GetJNIEnv(), obj, (jmethodID)this, l));
		break;
	case 'D':
		ret = toJValue(env.GetJNIEnv()->functions->CallDoubleMethodV(env.GetJNIEnv(), obj, (jmethodID)this, l));
		break;
	case '[':
	case 'L':
		ret = toJValue(env.GetJNIEnv()->functions->CallObjectMethodV(env.GetJNIEnv(), obj, (jmethodID)this, l));
		break;
	default:
		va_end(l);
		throw std::runtime_error("Unsupported signature");
	}
	va_end(l);
	return ret;
}

#define DeclareTemplate(T) template jvalue toJValue(T val);
DeclareTemplate(jboolean);
DeclareTemplate(jbyte);
DeclareTemplate(jshort);
DeclareTemplate(jint);
DeclareTemplate(jlong);
DeclareTemplate(jfloat);
DeclareTemplate(jdouble);
DeclareTemplate(jchar);
DeclareTemplate(jobject);
#undef DeclareTemplate