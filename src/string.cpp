#include <unicode/string.hpp>
#include <unicode/utf8.hpp>
#include <algorithm>
#include <cstring>


namespace Unicode {


	void String::trim_front (const Locale & locale) noexcept {
	
		auto iter=std::find_if(
			cps.begin(),
			cps.end(),
			[&] (CodePoint cp) noexcept {	return !cp.IsWhitespace(locale);	}
		);
		
		auto s=cps.size()-static_cast<std::size_t>(iter-cps.begin());
		
		std::memmove(
			&cps[0],
			&(*iter),
			s*sizeof(CodePoint)
		);
		
		cps.resize(s);
	
	}
	
	
	void String::trim_rear (const Locale & locale) noexcept {
	
		auto end=cps.rend();
		auto iter=std::find_if(
			cps.rbegin(),
			end,
			[&] (CodePoint cp) noexcept {	return !cp.IsWhitespace(locale);	}
		);
		
		cps.resize(static_cast<std::size_t>(end-iter));
	
	}
	
	
	static std::vector<CodePoint> from_c_string (const char * str) {
	
		return UTF8{}.Decode(
			str,
			str+std::strlen(str)
		).CodePoints();
	
	}
	
	
	String::String (const char * str) : cps(from_c_string(str)), locale(nullptr) {	}
	
	
	String & String::operator = (const char * str) {
	
		cps=from_c_string(str);
		locale=nullptr;
		
		return *this;
	
	}
	
	
	std::vector<char> String::ToCString () const {
	
		auto vec=UTF8{}.Encode(cps);
		vec.push_back('\0');
		
		return vec;
	
	}


	String String::Trim () const {
	
		String cpy=*this;
		
		cpy.Trim();
		
		return cpy;
	
	}
	
	
	String & String::Trim () noexcept {
	
		auto & locale=GetLocale();
	
		trim_front(locale);
		trim_rear(locale);
		
		return *this;
	
	}


}
