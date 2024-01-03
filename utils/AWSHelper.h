#ifndef _AWS_HELPER_H
#define _AWS_HELPER_H

#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <string>
#include <memory>
#include <aws/core/utils/memory/stl/AWSAllocator.h>
#include <aws/core/utils/memory/stl/AWSStringStream.h>

namespace Impl
{
	class API
	{
	public:
		API(const Aws::SDKOptions& _options);
		~API();
	private:
		const Aws::SDKOptions& options;
	};
} // Impl


class AWSHelper
{
 public:
	static AWSHelper& GetInstance();
	bool IsInit() { return isInit; }
	bool PutObject(const std::string& objectName, std::shared_ptr<Aws::IOStream> &body);
	static const char* BUCKET_NAME;
	static const char* ALLOC_TAG;
 private:
	AWSHelper(const std::string &bucketName);
	~AWSHelper();
	Aws::SDKOptions options;
	// NOTE: Do not change the order of this!
	Impl::API api;
	Aws::S3::S3Client client;
	bool isInit = false;
	std::string bucketName;
};

#endif // _AWS_HELPER_H
