#include <jni.h>
#include <string>
#include "md5.h"
#include "android/log.h"

using namespace std;
static char *EXTRA_SIGNATURE = "qqaazz";
static char *PACKAGE_NAME = "com.cnting.md5";
static char *APP_SIGNATURE = "308202e2308201ca020101300d06092a864886f70d010105050030373116301406035504030c0d416e64726f69642044656275673110300e060355040a0c07416e64726f6964310b3009060355040613025553301e170d3139303631343031333534325a170d3439303630363031333534325a30373116301406035504030c0d416e64726f69642044656275673110300e060355040a0c07416e64726f6964310b300906035504061302555330820122300d06092a864886f70d01010105000382010f003082010a02820101008634a31cbb236251577baa78d7e21e2ae2998a37489f9d80749d26f9228c0ec2165639c9c6c620b74e4c4059d8d18058bc6aec31200f85e208ab2e076f016f211637551e33e1cb71975c4ed3c477628000e049d7b093ed50847d839509e6bd1f401372aa142de174e3b86666fa045396c4bb358f5a0668409aa1cc10a40e4c6ec7cf7393f6c2d8a6afdff29db669f1dcf3947aeb9f92dece4fc3538f0ee830e94198bf11ef0271bd104c6514d3808ba98f1a753e6caec437ec5eac225ccc123c23916b83d476efdaedeafe89f0adcb37094b87c9a42bdcc4ed4ed54675c8c573f11e3ddb3b498bee82309c7eb92ed922aea7711843b534eb1f6b61441bc1225f0203010001300d06092a864886f70d010105050003820101001d6ad328b478b7bd17c58c119517d9908f54fde197f618b139e3aaf7e1383cc30d8118340c80f57d7fe333fcac4fd0684d7cd0e78efe52d2dbdee0213d72b7d85131e04ecef535638f5d8018e6bda846fcf8d6d2456964e6549783df4d4fe6edf72c71154755d6fd07f6f992f47e9c1c61b25db4d2a86d4a495b759adf43cf6227579f633a49c53ba4fb7e67090fac03d0c23721414eb8e180ed753f7ab64d9696657b7e9d02d5bb93a41b98090e0b9be95071e7b00c899d8fb1a8110f3f2868d17b54f0dc2d7dd04861e723ffaf7692c6f944d1902acb9dd0e7cb35e8c4231716c5397f5ad2ab5badbda50d4a0bb8f4a6a0d313298414e7b212f6eab308a91f";
static int is_verify = 0;

extern "C"
JNIEXPORT jstring JNICALL
Java_com_cnting_md5_SignatureUtils_signatureParams(JNIEnv *env, jobject thiz, jstring params) {
    if (is_verify == 0) {
        return env->NewStringUTF("error_signature");
    }
    //MD5签名规则，加点料
    //字符串前面加点料，后面去掉两位

    const char *cparams = env->GetStringUTFChars(params, JNI_FALSE);
    string signature_str(cparams);
    signature_str.insert(0, EXTRA_SIGNATURE);
    signature_str = signature_str.substr(0, signature_str.length() - 2);

    //md5加密
    signature_str = MD5(signature_str).toStr();

    env->ReleaseStringUTFChars(params, cparams);

    return env->NewStringUTF(signature_str.c_str());
}
/**
 * 签名校验
 *  PackageInfo packageInfo = context.getPackageManager().getPackageInfo(context.getPackageName(), PackageManager.GET_SIGNATURES);
    Signature[] signatures = packageInfo.signatures;
    return signatures[0].toCharsString();
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_cnting_md5_SignatureUtils_signatureVerify(JNIEnv *env, jobject thiz, jobject context) {
    //1.获取包名
    jclass j_clz = env->GetObjectClass(context);
    jmethodID j_mid = env->GetMethodID(j_clz, "getPackageName", "()Ljava/lang/String;");
    jstring j_package_name = (jstring) env->CallObjectMethod(context, j_mid);

    //2.比对包名是否一样
    const char *c_package_name = env->GetStringUTFChars(j_package_name, NULL);
    if (strcmp(c_package_name, PACKAGE_NAME) != 0) {
        return;
    }
    __android_log_print(ANDROID_LOG_ERROR, "JNI_TAG", "包名一致:%s", c_package_name);

    //3.获取签名
    //3.1 获取PackageManager
    j_mid = env->GetMethodID(j_clz, "getPackageManager", "()Landroid/content/pm/PackageManager;");
    jobject package_manager = env->CallObjectMethod(context, j_mid);
    //3.2 获取PackageInfo
    j_clz = env->GetObjectClass(package_manager);
    j_mid = env->GetMethodID(j_clz, "getPackageInfo",
                             "(Ljava/lang/String;I)Landroid/content/pm/PackageInfo;");
    jobject package_info = env->CallObjectMethod(package_manager, j_mid, j_package_name,
                                                 0x00000040);
    //3.3 获取Signature[]
    j_clz = env->GetObjectClass(package_info);
    jfieldID j_fid = env->GetFieldID(j_clz, "signatures", "[Landroid/content/pm/Signature;");
    jobjectArray signatures = static_cast<jobjectArray>(env->GetObjectField(package_info, j_fid));
    //3.4 获取signatre[0]
    jobject signature_first = env->GetObjectArrayElement(signatures, 0);
    //3.5 调用signatures[0].toCharsString()
    j_clz = env->GetObjectClass(signature_first);
    j_mid = env->GetMethodID(j_clz, "toCharsString", "()Ljava/lang/String;");
    jstring signature_str = static_cast<jstring>(env->CallObjectMethod(signature_first, j_mid));

    //4.比对签名是否一样
    const char *c_signature = env->GetStringUTFChars(signature_str, NULL);
    if (strcmp(c_signature, APP_SIGNATURE) != 0) {
        return;
    }
    __android_log_print(ANDROID_LOG_ERROR, "JNI_TAG", "签名一致:%s", c_signature);
    is_verify = 1;

}