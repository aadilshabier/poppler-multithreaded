#include "AWSHelper.h"
#include <aws/core/auth/AWSCredentialsProviderChain.h>

static const char* ALLOC_TAG = "pdftohtml";
static const char* BUCKET_NAME = "aadil-test-bucket";

AWSHelper::AWSHelper()
{
	Aws::InitAPI(options);
	auto provider = Aws::MakeShared<Aws::Auth::DefaultAWSCredentialsProviderChain>(ALLOC_TAG);
	auto creds = provider->GetAWSCredentials();
    isInit = !creds.IsEmpty();
}

AWSHelper::~AWSHelper()
{
	Aws::ShutdownAPI(options);
}

AWSHelper& AWSHelper::GetInstance()
{
	static AWSHelper instance;
	return instance;
}
