#include "usbspy.h"

using namespace Nan;

std::mutex m;
std::condition_variable cv;
bool ready = false;

void processData(const typename AsyncProgressQueueWorker<Deivce>::ExecutionProgress &progress);
v8::Local<v8::Value> Preparev8Object(const Deivce *data);

template <typename T>
class USBSpyWorker : public AsyncProgressQueueWorker<T>
{
  public:
	USBSpyWorker(Callback *callback, Callback *progress) : AsyncProgressQueueWorker<T>(callback), progress(progress)
	{
	}

	~USBSpyWorker()
	{
		delete progress;
	}

	void Execute(const typename AsyncProgressQueueWorker<T>::ExecutionProgress &progress)
	{
		std::unique_lock<std::mutex> lk(m);

		processData(progress);

		while (ready)
		{
			cv.wait(lk);
		}

		lk.unlock();
		cv.notify_one();
	}

	void HandleProgressCallback(const T *data, size_t count)
	{
		HandleScope scope;

		if (data)
		{
			v8::Local<v8::Value> obj = Preparev8Object(data);
			v8::Local<v8::Value> argv[] = {obj};

#if !defined(_DEBUG) || defined(_TEST_NODE_)
			progress->Call(1, argv);
#endif
		}
	}

  private:
	Callback *progress;
};

NAN_METHOD(SpyOn)
{
#if defined(_DEBUG) && !defined(_TEST_NODE_)
	Callback *progress = new Callback();
	Callback *callback = new Callback();
	Callback *notify = new Callback();
#else
	Callback *progress = new Callback(To<v8::Function>(info[0]).ToLocalChecked());
	Callback *callback = new Callback(To<v8::Function>(info[1]).ToLocalChecked());
	Callback *notify = new Callback(To<v8::Function>(info[2]).ToLocalChecked());
#endif

	AsyncQueueWorker(new USBSpyWorker<Deivce>(callback, progress));
#if defined(_TEST_NODE_) || !defined(_DEBUG)
	notify->Call(0, NULL); // notify everything is ready!
#endif
}

NAN_METHOD(SpyOff)
{
	{
		std::lock_guard<std::mutex> lk(m);
		ready = false;
	}
	cv.notify_one();
}

NAN_MODULE_INIT(Init)
{
	Set(target, New<v8::String>("spyOn").ToLocalChecked(), New<v8::FunctionTemplate>(SpyOn)->GetFunction());
	Set(target, New<v8::String>("spyOff").ToLocalChecked(), New<v8::FunctionTemplate>(SpyOff)->GetFunction());
	StartSpying();
}

v8::Local<v8::Value> Preparev8Object(const Deivce *data)
{
	v8::Local<v8::Object> device = Nan::New<v8::Object>();

	if (!data)
	{
		Nan::Set(
			device,
			Nan::New("device_status").ToLocalChecked(),
			New<v8::Number>(0));
		return device;
	}
        
    Nan::Set(device, Nan::New("name").ToLocalChecked(), New<v8::String>(data->szDeviceName).ToLocalChecked());
    Nan::Set(device, Nan::New("vid").ToLocalChecked(),  New<v8::Number>(data->dwVID));
    Nan::Set(device, Nan::New("pid").ToLocalChecked(),  New<v8::Number>(data->dwPID));
    Nan::Set(device, Nan::New("interface").ToLocalChecked(),  New<v8::Number>(data->dwInterface));
    Nan::Set(device, Nan::New("guid").ToLocalChecked(), New<v8::String>(data->szGuid).ToLocalChecked());
    Nan::Set(device, Nan::New("status").ToLocalChecked(), New<v8::Number>((int)data->status));

	return device;
}

void StartSpying()
{
	{
		std::lock_guard<std::mutex> lk(m);
		ready = true;
		std::cout << "Listening..." << std::endl;
	}
	cv.notify_one();

#if defined(_DEBUG) && !defined(_TEST_NODE_)
	New<v8::FunctionTemplate>(SpyOn)->GetFunction()->CallAsConstructor(0, {});
#endif
}

NODE_MODULE(hidspy, Init)