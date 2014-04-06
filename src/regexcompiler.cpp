#include <unicode/regex.hpp>
#include <unicode/regexcompiler.hpp>
#include <utility>


namespace Unicode {
	
	
	typedef std::vector<std::pair<std::size_t,const RegexParser *>> parsers_type;
	
	
	static parsers_type & get_parsers () {
	
		static parsers_type parsers;
		
		return parsers;
	
	}
	
	
	void RegexCompiler::Add (const RegexParser & parser, std::size_t priority) {
	
		auto & parsers=get_parsers();
	
		//	Find insertion point
		auto begin=parsers.begin();
		for (auto end=parsers.end();begin!=end;++begin) if (begin->first>priority) break;
		
		parsers.insert(begin,{priority,&parser});
	
	}
	
	
	RegexCompiler::Pattern RegexCompiler::operator () (
		const CodePoint * begin,
		const CodePoint * end,
		RegexOptions options,
		const Locale & locale
	) const {
	
		//	The pattern that is being compiled
		Pattern pattern;
		//	The last type of pattern element invoked
		const RegexParser * last=nullptr;
		//	The list of parsers
		auto & parsers=get_parsers();
		//	Prepare the state
		RegexCompilerState state(
			begin,
			end,
			options,
			//	If the culture invariant flag is set,
			//	the default locale is used regardless,
			//	otherwise whatever locale is provided is
			//	used
			Check(options,RegexOptions::CultureInvariant) ? DefaultLocale : locale
		);
		
		//	Loop over each code point in the input string
		while (state) {
		
			//	Track the start point so we can detect the
			//	case where no parser can generate a pattern
			//	element
			auto start=state.Begin;
			
			//	Loop over each parser
			for (auto & pair : parsers) {
			
				//	Make sure the state is clean
				state.Reset();
			
				//	The current parser
				auto & parser=*(pair.second);
				
				//	Attempt to parse using the preceding
				//	pattern element if this is the same type
				//	of parser as was used previously
				bool parsed=(pair.second==last) && parser(*(pattern.back().get()),state);
				//	If parsing above failed or was not performed,
				//	perform a regular parse -- i.e. attempt to create
				//	a new pattern element
				if (!parsed) {
				
					//	Rewind
					state.Begin=start;
				
					auto element=parser(state);
					if (element) {
					
						//	If applicable, complete last pattern element
						if (last!=nullptr) last->Complete(*(pattern.back().get()));
					
						//	Add newly-parsed pattern element
						parsed=true;
						pattern.push_back(std::move(element));
					
					}
				
				}
				
				//	If parsing was successful, perform cleanup
				//	and stop looping
				if (parsed || !state.Fail) {
				
					last=pair.second;
					//	Advance to next code point if parser
					//	failed to do so, to avoid possible
					//	infinite loop
					if (state.Begin==start) ++state.Begin;
					
					break;
				
				}
				
				//	Otherwise rewind and continue
				state.Begin=start;
			
			}
			
			//	If nothing was consumed, throw
			if (start==state.Begin) {
			
				//	TODO: Throw
			
			}
		
		}
		
		//	Complete the last pattern element
		if (last!=nullptr) last->Complete(*(pattern.back().get()));
		
		return pattern;
	
	}


}
