#ifndef UTILS_H
#define UTILS_H

#include <algorithm>
#include <iostream>
#include <string>

#define repeat_twice for (int _index_ = 0; _index_ < 2; ++_index_)

namespace EM {

	/** std::string trimming functions taken directly from stack overflow */

	/** trim from start */
	static inline std::string ltrimmed(const std::string &str)
	{
		std::string s(str);
		s.erase(s.begin(), std::find_if(s.begin(), s.end(),
		        std::not1(std::ptr_fun<int, int>(std::isspace))));
		return s;
	}

	/** trim from end */
	static inline std::string rtrimmed(const std::string &str)
	{
		std::string s(str);
		s.erase(std::find_if(s.rbegin(), s.rend(),
		        std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
		return s;
	}

	/** trim from both ends */
	static inline std::string trimmed(const std::string &s)
	{
		return ltrimmed(rtrimmed(s));
	}

	static inline std::string without_endline(const std::string &str)
	{
		std::string s(str);
		int i = s.size();
		while (i > 0 && s[i - 1] == '\n')
			--i;
		return s.substr(0, i);
	}

	namespace Default {
		static const uint NR_ALB = 334695;

		static const uint PORT = 10000 + NR_ALB % 10000;

		static const uint FIFO_SIZE           = 10560;
		static const uint FIFO_LOW_WATERMARK  = 0;
		static const uint FIFO_HIGH_WATERMARK = FIFO_SIZE;

		static const uint BUFFER_LENGTH    = 10;

		static const uint TX_INTERVAL = 5;

		const uint RETRANSMIT_LIMIT = 10;
	}
}

#endif // UTILS_H
