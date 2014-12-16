//
//  curl_error.h
//  curlcpp
//
//  Created by Giuseppe Persico on 02/06/14.
//

#ifndef __curlcpp__curl_error__
#define __curlcpp__curl_error__

#include <iostream>
#include <string>
#include <exception>
#include <utility>
#include <vector>

using std::cout;
using std::endl;
using std::string;
using std::exception;
using std::pair;
using std::make_pair;
using std::vector;

namespace curl {
class curl_error : public exception {
public:
	curl_error( const string, const string );
	~curl_error() throw() {
		/* ... nothing to delete */
	}
	using exception::what;
	const vector<pair<string, string>> what();
	const void print_traceback();
private:
	static vector<pair<string, string>> traceback;
};
}

#endif /* defined(__curlcpp__curl_error__) */