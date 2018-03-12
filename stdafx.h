// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <vector>
#include <chrono>
#include <cctype>
#include <iostream> 
#include <string>
#include <functional>
#include <fstream> 
#include <memory>
#include <stdexcept>
#include <array>

// TODO: reference additional headers your program requires here

#include "HttpListenerDefinitions.h"

#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/PutObjectRequest.h>
