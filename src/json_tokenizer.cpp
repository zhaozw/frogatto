#include <algorithm>
#include <ctype.h>
#include <string.h>

#include "json_tokenizer.hpp"
#include "unit_test.hpp"

namespace json {

Token get_token(const char*& i1, const char* i2)
{
	while(i1 != i2 && isspace(*i1) || *i1 == '#') {
		if(*i1 == '#') {
			//ignore comments.
			i1 = std::find(i1, i2, '\n');
		} else {
			++i1;
		}
	}

	if(i1 == i2) {
		Token result = { Token::NUM_TYPES, NULL, NULL };
		return result;
	}

	if(strchr("{}[]:,", *i1)) {
		Token result;
		result.begin = i1;
		result.end = i1+1;
		switch(*i1) {
			case '{': result.type = Token::TYPE_LCURLY; break;
			case '}': result.type = Token::TYPE_RCURLY; break;
			case '[': result.type = Token::TYPE_LSQUARE; break;
			case ']': result.type = Token::TYPE_RSQUARE; break;
			case ':': result.type = Token::TYPE_COLON; break;
			case ',': result.type = Token::TYPE_COMMA; break;
		}

		i1 = result.end;
		return result;
	}

	if(*i1 == '"' || *i1 == '\'') {
		const char quote_type = *i1;
		Token result;
		result.type = Token::TYPE_STRING;
		result.begin = ++i1;
		while(i1 != i2) {
			if(*i1 == quote_type) {
				break;
			} else if(*i1 == '\\') {
				++i1;
				if(i1 == i2) {
					break;
				}
			}

			++i1;
		}

		if(i1 == i2) {
			TokenizerError error = { "Unexpected end of file while parsing string", result.begin };
			throw error;
		}

		result.end = i1;
		++i1;
		return result;
	} else if(isalpha(*i1) || *i1 == '_') {
		Token result;
		result.begin = i1;
		while(i1 != i2 && (isalnum(*i1) || *i1 == '_')) {
			++i1;
		}

		result.end = i1;

		if(result.end - result.begin == 4 && !memcmp("true", result.begin, 4)) {
			result.type = Token::TYPE_TRUE_VALUE;
		} else if(result.end - result.begin == 5 && !memcmp("false", result.begin, 5)) {
			result.type = Token::TYPE_FALSE_VALUE;
		} else if(result.end - result.begin == 4 && !memcmp("null", result.begin, 4)) {
			result.type = Token::TYPE_NULL_VALUE;
		} else {
			result.type = Token::TYPE_IDENTIFIER;
		}

		return result;
	}

	if(*i1 == '-' || *i1 == '.' || isdigit(*i1)) {
		bool seen_decimal = false;
		Token result;
		result.type = Token::TYPE_NUMBER;
		result.begin = i1;
		while(i1 != i2) {
			if(*i1 == '.') {
				if(seen_decimal) {
					TokenizerError error = { "Two decimal points found in number", i1 };
					throw error;
					
				}

				seen_decimal = true;
			} else if(*i1 == '-') {
				if(i1 != result.begin) {
					TokenizerError error = { "- found in illegal position in number", i1 };
					throw error;
				}
			} else if(!isdigit(*i1)) {
				break;
			}

			++i1;
		}

		result.end = i1;
		return result;
	}

	TokenizerError error = { "Unexpected character found", i1 };
	throw error;
}

}