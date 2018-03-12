#include "HttpListenerThread.h"
#include <mutex> 

evutil_socket_t HttpListenerThread::socket=-1;
bool HttpListenerThread::active = true;
std::mutex mtx;

// Static callback handlers
void (*callbackHookWavInfo)(evhttp_request * request, void * args) = [](evhttp_request * request, void * args)
{
    HttpListenerThread* instance = static_cast<HttpListenerThread*>(args);
    instance->httpWavInfoCallback(request, args);
};

void (*callbackHookMp3ToWav)(evhttp_request * request, void * args) = [](evhttp_request * request, void * args)
{
    HttpListenerThread* instance = static_cast<HttpListenerThread*>(args);
    instance->httpMp3ToWavCallback(request, args);
};

void (*callbackHook)(evhttp_request * request, void * args) = [](evhttp_request * request, void * args)
{
    HttpListenerThread* instance = static_cast<HttpListenerThread*>(args);
    instance->httpEventCallback(request, args);
};

// Class implementation 
void HttpListenerThread::operator() (unsigned int port, unsigned int threadID)
{
    printf("Thread %d listening on port %d... \n", threadID, port);

    // Created libevent event base
    std::unique_ptr<event_base, std::function<void(event_base *)>> eventBase(event_base_new(), &event_base_free);
    if(!eventBase)
    {
        printf("Thread %d ERROR: Failed to create event base!\n", threadID);
        return;
    }

    // Create libevent HTTP request handler
    std::unique_ptr<evhttp, std::function<void(evhttp *)>> httpEvent(evhttp_new(eventBase.get()), &evhttp_free);
    if(!httpEvent)
    {
        printf("Thread %d ERROR: Failed to create http event handler!\n", threadID);
        return;
    }

    // Initialize AWS
    Aws::SDKOptions options;
    Aws::InitAPI(options);

    // Register specific URI and general HTTP callback functions.
    evhttp_set_cb(httpEvent.get(), "/wav-info", callbackHookWavInfo, (void*)this);
    evhttp_set_cb(httpEvent.get(), "/mp3-to-wav", callbackHookMp3ToWav, nullptr);
    evhttp_set_gencb(httpEvent.get(), callbackHook, nullptr);

    // Bind to listening socket
    mtx.lock();
    if(socket == -1)
    {
        boundSocket=evhttp_bind_socket_with_handle(httpEvent.get(), SERVER_IP, port);
        if(!boundSocket)
        {
            printf("Thread %d ERROR: Failed to bind socket with handle!\n", threadID);
            return;
        }
        if ((socket = evhttp_bound_socket_get_fd(boundSocket)) == -1)
        {
            printf("Thread %d ERROR: Failed to get fd!\n", threadID);
            return;
        }
    }
    else
    {
        if (evhttp_accept_socket(httpEvent.get(), socket) == -1)
        printf("Thread %d ERROR: Failed to bind socket.", threadID);
    }
    mtx.unlock();

    // Check if there are HTTP requests
    while(active)
    {
        event_base_loop(eventBase.get(), EVLOOP_NONBLOCK);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    // AWS shutdown
    Aws::ShutdownAPI(options);
    printf("Thread %d EXIT OK\n", threadID);
}

void HttpListenerThread::deactivate()
{
    HttpListenerThread::active = false;
}

void HttpListenerThread::httpEventCallback(evhttp_request * request, void * args)
{
    evbuffer *outputBuffer = evhttp_request_get_output_buffer(request);
    if (outputBuffer)
    {
        evbuffer_add_printf(outputBuffer, "<html><body><center><h1>Request received. Processing not available for this type of request.</h1></center></body></html>");
        evhttp_send_reply(request, HTTP_OK, "", outputBuffer);
    }
}

void HttpListenerThread::httpWavInfoCallback(evhttp_request * request, void * args)
{
    evbuffer *outputBuffer = evhttp_request_get_output_buffer(request);
    std::string requestUri = "";

    if (outputBuffer)
    {
        requestUri.assign(request->uri);

    	std::string wavInfoKey = parseKey(requestUri, "wavkey");
    	std::string wavFileName = getFile("uar-patrick-code-test", wavInfoKey);

    	std::string command = "soxi -c ";
    	command.append(wavFileName);
    	std::string channelNo = executeCommand(command.c_str());
    	command = "soxi -r ";
    	command.append(wavFileName);
    	std::string sampleRate = executeCommand(command.c_str());
    	command = "soxi -D ";
    	command.append(wavFileName);
    	std::string duration = executeCommand(command.c_str());

        std::string prnt = "{channel_count: ";
        prnt.append(channelNo);
        prnt.append(", sample_rate: ");
        prnt.append(sampleRate);
        prnt.append(", execution_time: ");
        prnt.append(duration);
        prnt.append("}");

        evbuffer_add_printf(outputBuffer, prnt.c_str());
        evhttp_send_reply(request, HTTP_OK, "", outputBuffer);
    }
}

void HttpListenerThread::httpMp3ToWavCallback(evhttp_request * request, void * args)
{
    evbuffer *outputBuffer = evhttp_request_get_output_buffer(request);
    std::string requestUri = "";

    if (outputBuffer)
    {
        requestUri.assign(request->uri);

    	std::string mp3KeyValue = parseKey(requestUri, "mp3key");
    	std::string mp3FileName = getFile("uar-patrick-code-test", mp3KeyValue);

    	std::string wavKeyValue = parseKey(requestUri, "wavkey");
    	std::string wavFileName = getFileName(wavKeyValue);

    	std::string command = "sox ";
    	command.append(mp3FileName);
    	command.append(" ");
    	command.append(wavFileName);
    	std::system(command.c_str());
    	command = "soxi -D ";
    	command.append(wavFileName);
    	std::string duration = executeCommand(command.c_str());
    	command = "du -k ";
    	command.append(wavFileName);
    	command.append(" | cut -f1");
    	std::string fileSize = executeCommand(command.c_str());

    	uploadFile(wavKeyValue, wavFileName);
    	std::string prnt = "{file_size: ";
        prnt.append(fileSize);
        prnt.append(", execution_time: ");
        prnt.append(duration);
        prnt.append("}");

        evbuffer_add_printf(outputBuffer, prnt.c_str());
        evhttp_send_reply(request, HTTP_OK, "", outputBuffer);
    }
}

std::string HttpListenerThread::parseKey(std::string &uri, const std::string &keyname)
{
    size_t keyPos = uri.find(keyname);
    size_t ampersendPos = uri.find("&");
    int startPos = keyPos + keyname.length() + 1;
    std::string result = "";

    if((ampersendPos == std::string::npos) || (ampersendPos<keyPos))
    {
        return uri.substr(startPos);
    }
    else
    {
        int length = ampersendPos - startPos;
        std::cout << "Length: " << length << std::endl;
        return uri.substr(startPos, length);
    }
}

std::string HttpListenerThread::getFile(std::string bucketName, std::string keyName)
{
    std::cout << "Downloading " << keyName << " from S3 bucket: " << bucketName << std::endl;

    Aws::Client::ClientConfiguration config;
    config.region = "us-east-2"; 
    Aws::S3::S3Client s3Client(config);
    Aws::S3::Model::GetObjectRequest objectRequest;
    objectRequest.WithBucket(bucketName.c_str()).WithKey(keyName.c_str());
    auto result = s3Client.GetObject(objectRequest);
    Aws::OFStream downloadFile;

    if (result.IsSuccess())
    {
        std::string path="./";
        path.append(getFileName(keyName));
        downloadFile.open(path.c_str(), std::ios::out | std::ios::binary);
        downloadFile << result.GetResult().GetBody().rdbuf();
        std::cout << "Done!" << std::endl;
        downloadFile.close();
        return path;
    }
    else
    {
        std::cout << "GetObject error: " << result.GetError().GetExceptionName() << " " << result.GetError().GetMessage() << std::endl;
    }
    return "";
}

std::string HttpListenerThread::executeCommand(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), 128, pipe.get()) != nullptr)
            result += buffer.data();
    }
    return result;
}

std::string HttpListenerThread::getFileName(std::string keyName)
{
    size_t delimiter = keyName.find_last_of("/");
    std::string path;
    if(delimiter != std::string::npos)
    {
        path = keyName.substr(delimiter+1);
    }
    else
    {
        path = keyName;
    }
    return path;
}

bool HttpListenerThread::uploadFile(std::string wavKeyValue, std::string  wavFileName)
{
    const Aws::String bucket_name = argv[1];
        const Aws::String key_name = argv[2];
        const Aws::String file_name = argv[3];
        const Aws::String region(argc > 4 ? argv[4] : "");

        std::cout << "Uploading " << file_name << " to S3 bucket " <<
            bucket_name << " at key " << key_name << std::endl;

        Aws::Client::ClientConfiguration clientConfig;
        if (!region.empty())
            clientConfig.region = region;
        Aws::S3::S3Client s3_client(clientConfig);

        Aws::S3::Model::PutObjectRequest object_request;
        object_request.WithBucket(bucket_name).WithKey(key_name);

        // Binary files must also have the std::ios_base::bin flag or'ed in
        auto input_data = Aws::MakeShared<Aws::FStream>("PutObjectInputStream",
            file_name.c_str(), std::ios_base::in | std::ios_base::binary);

        object_request.SetBody(input_data);

        auto put_object_outcome = s3_client.PutObject(object_request);

        if (put_object_outcome.IsSuccess())
        {
            std::cout << "Done!" << std::endl;
        }
        else
        {
            std::cout << "PutObject error: " <<
                put_object_outcome.GetError().GetExceptionName() << " " <<
                put_object_outcome.GetError().GetMessage() << std::endl;
}
}