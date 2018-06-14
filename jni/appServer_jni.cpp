#define LOG_TAG "appServer_jni"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>

#include <assert.h>
#include <errno.h>
#include <unistd.h>

#include <utils/Log.h>
#include <nativehelper/jni.h>
#include <nativehelper/JNIHelp.h>
#include <android_runtime/AndroidRuntime.h>
#include <android_runtime/Log.h>

#include "json/json.h"

using namespace android;

#define APP_OK		0
#define APP_FAIL	-1

#define NETWORK_DEVICE "eth0"		/**< 网卡名称 */
#define CGI_SOCKET_UNIX_DOMAIN  "/system/www/cgi-bin/cgi_socket_unix_domain" /**< CGI与应用服务器通信的本地socket文件名 */

#define LOCAL_SOCKET_TIMEOUT 150
#define LOCAL_SOCKET_HEADER_MAGIC 		0xaabbcc	/**< 包头 */

typedef struct local_socket_header {
    unsigned int magic;
    unsigned int length;
    char data[0];
} local_socket_header_t;

ssize_t read_timeout(int fd, void *buf, size_t count, int seconds)
{
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(fd,&fds);
	struct timeval timeval;
	timeval.tv_sec = seconds;
	timeval.tv_usec = 0;
	int ret = select(fd+1,&fds, NULL, NULL,&timeval);
	if(ret > 0) {
		return read(fd, buf, count);
	}
	else if (ret == -1){
		return -1;
	}
	else {
		return 0;
	}
	
	return 0;
}


int unix_socket_send(std::string &request, std::string &reply)
{
	int socketfd = socket(PF_UNIX,SOCK_STREAM,0);
	if(socketfd < 0) {
		return -APP_FAIL;
	}

	struct sockaddr_un client_addr;
    memset(&client_addr, 0x0, sizeof(struct sockaddr_un));
    client_addr.sun_family = AF_UNIX;
    strncpy(client_addr.sun_path, CGI_SOCKET_UNIX_DOMAIN, strlen(CGI_SOCKET_UNIX_DOMAIN)+1);

	int ret = connect(socketfd,(struct sockaddr*)&client_addr,sizeof(client_addr)); 
	if(ret < 0) {
		return APP_FAIL;
	}

	char buffer[1024] = {0};
	local_socket_header_t *local_request = (local_socket_header_t*)buffer;
	local_request->magic = LOCAL_SOCKET_HEADER_MAGIC;
	local_request->length = request.length() + 1;
	request.copy(local_request->data, 1024, 0);
	ret = write(socketfd, buffer, sizeof(local_socket_header_t)+local_request->length);
	if(ret < 0) {
		return APP_FAIL;
	}

	struct local_socket_header response_header;
	bzero(&response_header, sizeof(struct local_socket_header));
	ret = read_timeout(socketfd, &response_header, sizeof(struct local_socket_header), LOCAL_SOCKET_TIMEOUT);
	bzero(buffer, 1024);

	ret = read(socketfd, buffer, response_header.length);
	if(ret != response_header.length) {
		ALOGW("read data error!");
		return APP_FAIL;
	}

	reply = buffer;
	return APP_OK;
}

int video_hdmi_inner_enable(JNIEnv *env, jobject obj, int channel, jboolean enable)
{
	ALOGW("video_hdmi_inner_enable(%d, %d)", channel, enable);
	Json::Value jsoRequest;
	Json::Value jsoData;	
	
	jsoRequest["method"] = "update";
	jsoRequest["object"] = "controlPenal";
	jsoRequest["subObject"] = (1==channel)?"voiceCall":"remoteInteracton";
	jsoRequest["apiVersion"] = "v1.0";

	jsoData["trigger"] = enable;
	jsoRequest["data"] = jsoData;	
	//ALOGW("request: %s", jsoRequest.toStyledString().c_str());
	
	Json::FastWriter fwriter;
	std::string request = fwriter.write(jsoRequest);
	ALOGW("request: %s", request.c_str());
	std::string reply;

	int ret = unix_socket_send(request, reply);
	if(ret < 0) {
		return APP_FAIL;
	}
	ALOGW("reply: %s", reply.c_str());

	Json::Reader reader;
	Json::Value jsoReply;

	if(!reader.parse(reply, jsoReply)) {
		return APP_FAIL;
	}

	if(0 !=jsoReply["code"].asInt()) {
		return APP_FAIL;
	}
	
	return APP_OK;
}


static JNINativeMethod gMethods[] = {
	{
		"videoHdmiInnerEnable",
		"(IZ)I",
		(void*)video_hdmi_inner_enable
	}
};

static int registerNativeMethods(JNIEnv* env, const char* className, JNINativeMethod* gMethods, int numMethods) {
    jclass clazz;

    clazz = env->FindClass(className);                                                                                                                                                                                                                                        
    if (clazz == NULL) {
        ALOGE("Native registration unable to find class '%s'", className);
        return JNI_FALSE;
    }
	
    if (env->RegisterNatives(clazz, gMethods, numMethods) < 0) {
        ALOGE("RegisterNatives failed for '%s'", className);
        return JNI_FALSE;
    }

    return JNI_TRUE;
}


jint JNI_OnLoad(JavaVM *vm, void *reserved) 
{
	JNIEnv* env = NULL; 																																																													  
	
	if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
		ALOGE("ERROR: GetEnv failed\n");
		return JNI_FALSE; 
	}	
	assert(env != NULL);

	if (!registerNativeMethods(env, "TestJni", gMethods, sizeof(gMethods) / sizeof(gMethods[0]))) {
        return JNI_FALSE;
    }

	return JNI_VERSION_1_4;
}

//_android_log_print(ANDROID_LOG_ERROR, "hello", "livingstone");


