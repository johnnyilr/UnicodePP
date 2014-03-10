#include <unicode/regex.hpp>
#include <unicode/regexcompiler.hpp>


namespace Unicode {


	Regex::Regex (const String & pattern, RegexOptions options, const Locale & locale)
		:	pattern(RegexCompiler{}(
				pattern.begin(),
				pattern.end(),
				options,
				locale
			))
	{	}
	
	
	Regex::Regex (const CodePoint * begin, const CodePoint * end, RegexOptions options, const Locale & locale)
		:	pattern(RegexCompiler{}(
				begin,
				end,
				options,
				locale
			))
	{	}


}
