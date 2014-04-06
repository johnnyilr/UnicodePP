#include <unicode/caseconverter.hpp>
#include <unicode/codepoint.hpp>
#include <unicode/makereverseiterator.hpp>
#include <unicode/regex.hpp>
#include <unicode/regexcompiler.hpp>
#include <unicode/vector.hpp>
#include <cstddef>
#include <limits>
#include <optional>
#include <utility>


namespace Unicode {


	namespace {
	
	
		class RegexLiteral : public RegexPatternElement {
		
		
			private:
			
			
				std::vector<CodePoint> cps;
				
				
				template <typename Iterator>
				static bool case_sensitive_check (Iterator begin, Iterator end, RegexState & state) noexcept {
				
					for (;begin!=end;++begin,++state) {
					
						if (!(state && (*state==*begin))) return false;
					
					}
					
					return true;
				
				}
				
				
				template <typename Iterator1, typename Iterator2>
				static bool check (Iterator1 & begin_a, Iterator1 & end_a, Iterator2 begin_b, Iterator2 end_b) noexcept {
				
					for (;(begin_a!=end_a) && (begin_b!=end_b);++begin_a,++begin_b) {
					
						if (*begin_a!=*begin_b) return false;
						
					}
					
					return begin_b==end_b;
				
				}
				
				
				template <typename Iterator>
				bool case_insensitive_check (Iterator begin, Iterator end, RegexState & state) const noexcept {
				
					auto b=state.Begin();
					auto e=state.End();
					CaseConverter cc(Locale);
					for (;begin!=end;++state) {
					
						if (!state) return false;
					
						auto folded=cc.Fold(&(*state),b,e);
						
						if (!(
							state.Reversed() ? check(
								begin,
								end,
								MakeReverseIterator(folded.end()),
								MakeReverseIterator(folded.begin())
							) : check(
								begin,
								end,
								folded.begin(),
								folded.end()
							)
						)) return false;
					
					}
					
					return true;
				
				}
				
				
				template <typename Iterator>
				bool check (Iterator begin, Iterator end, RegexState & state) const noexcept {
				
					return Check(Options,RegexOptions::IgnoreCase) ? case_insensitive_check(
						std::move(begin),
						std::move(end),
						state
					) : case_sensitive_check(
						std::move(begin),
						std::move(end),
						state
					);
				
				}
		
		
			public:
			
			
				using RegexPatternElement::RegexPatternElement;
				
				
				virtual bool operator () (RegexState & state, RegexPatternElementState &) const override {
				
					return state.Reversed() ? check(
						MakeReverseIterator(cps.end()),
						MakeReverseIterator(cps.begin()),
						state
					) : check(
						cps.begin(),
						cps.end(),
						state
					);
				
				}
				
				
				virtual RegexToString ToString () const override {
				
					RegexToString retr;
					retr.Parent << "The literal string: \"" << cps << "\"";
					
					return retr;
				
				}
				
				
				void Add (CodePoint cp) {
				
					cps.push_back(cp);
				
				}
				
				
				void Complete () {
				
					//	If not ignoring case, we're done
					if (!Check(Options,RegexOptions::IgnoreCase)) return;
					
					//	Case fold
					cps=CaseConverter(Locale).Fold(Begin(cps),End(cps));
				
				}
		
		
		};
	
	
		class RegexLiteralParser : public RegexParser {
		
		
			private:
			
			
				static std::optional<CodePoint> get (const CodePoint * & begin, const CodePoint * end, std::size_t num, std::size_t base=16) {
				
					//	If there are no characters left, fail
					if (begin==end) return std::nullopt;
				
					//	Keep track of the start point in case we
					//	need to rewind
					auto start=begin;
					
					//	Add each character
					std::size_t i=0;
					String s;
					for (;(i<num) && (begin!=end);++i) s << *(begin++);
					
					//	If the requested number of characters are not
					//	taken, fail
					if (i!=num) {
					
						begin=start;
						
						return std::nullopt;
					
					}
					
					//	Attempt conversion to a code point
					auto cp=s.To<CodePoint::Type>(base);
					if (!cp) {
					
						begin=start;
						
						return std::nullopt;
					
					}
					
					return CodePoint(*cp);
				
				}
				
				
				static std::size_t get_num (CodePoint cp) noexcept {
				
					switch (cp) {
					
						case 'x':
							return 2;
						case 'u':
							return 4;
						case 'U':
							return 8;
						default:
							return 5;
					
					}
				
				}
			
			
				static CodePoint get (const CodePoint * & begin, const CodePoint * end) {
				
					//	If the current code point is not a backslash, just
					//	return the current code point
					if (*begin!='\\') return *begin;
					
					//	If the current code point is a backslash, there is
					//	custom handling -- the backslash begins an escape
					//	sequence
					
					++begin;
					
					//	If the backslash is at the end of the string, return
					//	the literal backslash
					if (begin==end) return '\\';
					
					//	Get the next character from the stream
					auto next=*(begin++);
					
					switch (next) {
					
						case 'a':
							return 0x7;
						case 'b':
							return 0x8;
						case 't':
							return 0x9;
						case 'r':
							return 0xD;
						case 'v':
							return 0xB;
						case 'f':
							return 0xC;
						case 'n':
							return 0xA;
						case 'e':
							return 0x1B;
						//	Various hexadecimal escapes
						case 'x':
						case 'u':
						case 'U':{
							auto start=begin;
							auto cp=get(begin,end,get_num(next));
							if (cp) return *cp;
							begin=start;
							break;
						}
						//	Octal escape
						case '0':
						case '1':
						case '2':
						case '3':
						case '4':
						case '5':
						case '6':
						case '7':{
							auto start=begin;
							--begin;
							auto cp=get(begin,end,3,8);
							if (cp) return *cp;
							begin=start;
							break;
						}
						default:
							break;
							
					}
					
					//	If all else fails, take the current code point
					//	literally
					return next;
				
				}
		
		
			public:
			
			
				virtual RegexCompiler::Element operator () (RegexCompilerState & state) const override {
				
					//	This is safe because the constructor for std::unique_ptr
					//	is noexcept
					auto ptr=new RegexLiteral(state.Options,state.Locale);
					RegexCompiler::Element retr(ptr);
					
					ptr->Add(get(state.Current,state.End));
					
					return retr;
				
				}
				
				
				virtual bool operator () (RegexPatternElement & element, RegexCompilerState & state) const override {
				
					reinterpret_cast<RegexLiteral &>(element).Add(get(state.Current,state.End));
					
					return true;
				
				}
				
				
				virtual void Complete (RegexPatternElement & element) const override {
				
					reinterpret_cast<RegexLiteral &>(element).Complete();
				
				}
		
		
		};
	
	
	}
	
	
	static RegexParserInstaller<RegexLiteralParser> installer(std::numeric_limits<std::size_t>::max());


}
