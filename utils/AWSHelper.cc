#include "AWSHelper.h"
#include <iostream>
#include <aws/core/auth/AWSCredentialsProviderChain.h>
#include <aws/s3/model/HeadBucketRequest.h>
#include <aws/s3/model/PutObjectRequest.h>

extern char bucketNameArg[64];
const char* AWSHelper::ALLOC_TAG = "pdftohtml";

extern bool printCommands;

Impl::API::API(const Aws::SDKOptions& _options)
	: options(_options)
{
	Aws::InitAPI(options);
}

Impl::API::~API()
{
	Aws::ShutdownAPI(options);
}

AWSHelper::AWSHelper(const std::string& _bucketName)
	: api(options)
	, bucketName(_bucketName)
{
	auto provider = Aws::MakeShared<Aws::Auth::DefaultAWSCredentialsProviderChain>(ALLOC_TAG);
	auto creds = provider->GetAWSCredentials();
	isInit = !creds.IsEmpty();
	if (!isInit) {
		std::cerr << "ERROR: Could not authenticate!" << std::endl;
		return;
	}

	// Check if bucket exists
	Aws::S3::Model::HeadBucketRequest request;
	request.SetBucket(bucketName);
	auto outcome = client.HeadBucket(request);
	isInit = outcome.IsSuccess();
	if (!isInit) {
		auto error = outcome.GetError();
		Aws::String errorMessage = "ERROR: Bucket does not exist: " + error.GetExceptionName() + " - " + error.GetMessage();
		std::cerr << errorMessage << std::endl;
		return;
	}

	if (printCommands)
		std::cout << "Connected to bucket: " << bucketName << std::endl;
}

AWSHelper::~AWSHelper() = default;

AWSHelper& AWSHelper::GetInstance()
{
	static AWSHelper instance(bucketNameArg);
	return instance;
}

bool AWSHelper::PutObject(const std::string& objectName, std::shared_ptr<Aws::IOStream> &body)
{
	// if (printCommands)
		// std::cout << "PutObject: " << objectName << std::endl;
	// return true;

	Aws::S3::Model::PutObjectRequest request;
	request.SetBucket(bucketName);
	request.SetKey(objectName);
	request.SetBody(body);

	auto outcome = client.PutObject(request);
	if (!outcome.IsSuccess()) {
        std::cerr << "ERROR: PutObject: " <<
			outcome.GetError().GetMessage() << std::endl;
    }
    else {
		if (printCommands)
			std::cout << "PutObject: " << objectName << std::endl;
    }

    return outcome.IsSuccess();
}
