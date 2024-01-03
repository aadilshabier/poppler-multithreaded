#include "AWSHelper.h"
#include <iostream>
#include <aws/core/auth/AWSCredentialsProviderChain.h>
#include <aws/s3/model/HeadBucketRequest.h>
#include <aws/s3/model/PutObjectRequest.h>

const char* AWSHelper::BUCKET_NAME = "aadil-test-bucket";
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
	Aws::Client::ClientConfiguration clientConfig;
	auto provider = Aws::MakeShared<Aws::Auth::DefaultAWSCredentialsProviderChain>(ALLOC_TAG);
	auto creds = provider->GetAWSCredentials();
	isInit = !creds.IsEmpty();
	if (!isInit) {
		std::cerr << "ERROR: Could not authenticate!" << std::endl;
		return;
	}

	client = Aws::S3::S3Client(clientConfig);

	// Check if bucket exists
	Aws::S3::Model::HeadBucketRequest request;
	request.SetBucket(bucketName);
	auto outcome = client.HeadBucket(request);
	isInit = outcome.IsSuccess();
	if (!isInit) {
		auto error = outcome.GetError();
		Aws::String errorMessage = "ERROR: " + error.GetExceptionName() + " - " + error.GetMessage();
		std::cerr << errorMessage << std::endl;
		return;
	}

	if (printCommands)
		std::cout << "Connected to bucket: " << BUCKET_NAME << std::endl;
}

AWSHelper::~AWSHelper() = default;

AWSHelper& AWSHelper::GetInstance()
{
	static AWSHelper instance(BUCKET_NAME);
	return instance;
}

bool AWSHelper::PutObject(const std::string& objectName, std::shared_ptr<Aws::IOStream> &body)
{
	if (printCommands)
		std::cout << "PutObject: " << objectName << std::endl;
	return true;

	/*
	Aws::S3::Model::PutObjectRequest request;
	request.SetBucket(BUCKET_NAME);
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
	*/
}
