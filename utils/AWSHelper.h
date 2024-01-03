#ifndef _AWS_HELPER_H
#define _AWS_HELPER_H

#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>

class AWSHelper
{
 public:
	AWSHelper();
	~AWSHelper();
	static AWSHelper& GetInstance();
	bool IsInit() { return isInit; }
 private:
	Aws::SDKOptions options;
	Aws::S3::S3Client s3Client;
	bool isInit = false;
};

#endif // _AWS_HELPER_H
