#pragma once

#include "string.h"
#include "dynarray.hpp"
#include <compare>

#ifdef FP_OSTREAM_SUPPORT
	#include <ostream>
	#include <string_view>
#endif

#if __cpp_lib_format >= 201907L
	#include <format>
	#include <string_view>
#endif

namespace fp {

	inline static size_t encode_utf8(uint32_t codepoint, char out[4]) { return fp_encode_utf8(codepoint, out); }

	struct string_view: public view<char> {
		static string_view from_cstr(const char* str) { return {fp_string_view_literal((char*)str, fp_string_length(str))}; }

		struct string make_dynamic() const;
		int compare(const string_view o) const;
		dynarray<uint32_t> to_codepoints() const;
		struct string replicate(size_t times) const;
		struct string format(...) const;

		struct string replace_range(const string_view with, size_t start, size_t len) const;
		size_t replace_first(const string_view find, const string_view replace, size_t start = 0) const;
		struct string replace(const string_view find, const string_view replace, size_t start = 0) const;

		inline string_view subview(size_t start, size_t length) { return {view<char>::subview(start, length)}; }
		inline string_view subview_max_size(size_t start, size_t length) { return {view<char>::subview_max_size(start, length)}; }
		inline string_view subview(size_t start) { return {view<char>::subview(start)}; }
		inline string_view subview_start_end(size_t start, size_t end) { return {view<char>::subview_start_end(start, end)}; }

		std::strong_ordering operator<=>(const string_view o) const {
			auto res = compare(o);
			if(res < 0) return std::strong_ordering::less;
			if(res > 0) return std::strong_ordering::greater;
			return std::strong_ordering::equal;
		}
		std::strong_ordering operator<=>(const char* o) const {
			auto res = compare({fp_string_to_view_const(o)});
			if(res < 0) return std::strong_ordering::less;
			if(res > 0) return std::strong_ordering::greater;
			return std::strong_ordering::equal;
		}
		bool operator==(const string_view o) const { return this->operator<=>(o) == std::strong_ordering::equal; }
		bool operator==(const char* o) const { return this->operator<=>(o) == std::strong_ordering::equal; }

		inline size_t find(const string_view needle, size_t start = 0) const { return fp_string_view_find(view_(), needle, start); }
		bool contains(const string_view needle, size_t start = 0) const { return fp_string_view_contains(view_(), needle, start); }
		
		bool starts_with(const string_view needle, size_t start = 0) const { return fp_string_view_starts_with(view_(), needle, start); }
		bool ends_with(const string_view needle, size_t end = 0) const { return fp_string_view_ends_with(view_(), needle, end); }
		
		inline fp::dynarray<string_view> split(const string_view delimiters) const { return {(string_view*)fp_string_view_split(view_(), delimiters)}; }

#ifdef FP_OSTREAM_SUPPORT
		std::string_view to_std() { return {data(), size()}; }
		friend std::ostream& operator<<(std::ostream& s, const string_view& view) {
			return s << const_cast<string_view&>(view).to_std();
		}
#endif
#if __cpp_lib_format >= 201907L
		friend std::formatter<string_view, char>;
#endif
	};

	template<typename Derived, typename Dynamic>
	struct string_crtp_common {
		constexpr static size_t npos = fp_string_npos;

		static Dynamic from_codepoints(const fp::view<const uint32_t> codepoints) { return fp_codepoints_view_to_string((fp::view<uint32_t>&)codepoints); }
		static Dynamic from_codepoints(const dynarray<const uint32_t> codepoints) { return fp_codepoints_to_string(codepoints.raw); }

		static Dynamic make_dynamic(const char* string) { return fp_string_make_dynamic(string); }
		Dynamic make_dynamic() const { return make_dynamic(ptr()); }
		void free() { fp_string_free(ptr()); }
		void free_and_null() { fp_string_free_and_null(ptr()); }

		string_view to_view() { return {fp_string_to_view(ptr())}; }
		const string_view to_view() const { return {fp_string_to_view_const(ptr())}; }

		size_t length() const { return fp_string_length(ptr()); }
		size_t size() const { return length(); }
		int compare(const char* o) const { return fp_string_compare(ptr(), o); }
		int compare(const string_view o) const { return fp_string_view_compare(to_view(), o); }
		std::strong_ordering operator<=>(const string_view o) const {
			auto res = compare(o);
			if(res < 0) return std::strong_ordering::less;
			if(res > 0) return std::strong_ordering::greater;
			return std::strong_ordering::equal;
		}
		std::strong_ordering operator<=>(const char* o) const {
			auto res = compare(o);
			if(res < 0) return std::strong_ordering::less;
			if(res > 0) return std::strong_ordering::greater;
			return std::strong_ordering::equal;
		}
		std::strong_ordering operator<=>(const Derived& o) const { return *this <=> o.data(); }
		bool operator==(const string_view o) const { return this->operator<=>(o) == std::strong_ordering::equal; }
		// bool operator==(const char* o) const { return this->operator<=>(o) == std::strong_ordering::equal; }
		friend bool operator==(const Derived& a, const Derived& b) { return a.operator<=>(b) == std::strong_ordering::equal; }

		dynarray<uint32_t> to_codepoints() const { return fp_string_to_codepoints(ptr()); }

		size_t find(const string_view needle, size_t start = 0) const { return fp_string_view_find(to_view(), needle, start); }
		size_t find(const char* needle, size_t start = 0) const { return fp_string_find(ptr(), needle, start); }
		bool contains(const string_view needle, size_t start = 0) const { return fp_string_view_contains(to_view(), needle, start); }
		bool contains(const char* needle, size_t start = 0) const { return fp_string_contains(ptr(), needle, start); }
		
		bool starts_with(const string_view needle, size_t start = 0) const { return fp_string_view_starts_with(to_view(), needle, start); }
		bool starts_with(const char* needle, size_t start = 0) const { return fp_string_starts_with(ptr(), needle, start); }
		bool ends_with(const string_view needle, size_t end = 0) const { return fp_string_view_ends_with(to_view(), needle, end); }
		bool ends_with(const char* needle, size_t end = 0) const { return fp_string_ends_with(ptr(), needle, end); }

		fp::dynarray<string_view> split(const string_view delimiters) { return {(string_view*)fp_string_view_split(to_view(), delimiters)}; }
		fp::dynarray<const string_view> split(const string_view delimiters) const { return {(const string_view*)fp_string_view_split(to_view(), delimiters)}; }
		fp::dynarray<string_view> split(const char* delimiters) { return {(string_view*)fp_string_split(ptr(), delimiters)}; }
		fp::dynarray<const string_view> split(const char* delimiters) const { return {(const string_view*)fp_string_split(ptr(), delimiters)}; }

		Dynamic replace_range(const string_view with, size_t start, size_t len) const {
			return fp_string_view_replace_range(to_view(), with, start, len);
		}
		Dynamic replace_range(const char* with, size_t start, size_t len) const {
			return fp_string_replace_range(ptr(), with, start, len);
		}
		size_t replace_first(const string_view find, const string_view replace, size_t start = 0) const {
			return fp_string_view_replace_first(to_view(), find, replace, start);
		}
		size_t replace_first(const char* find, const char* replace, size_t start = 0) const {
			return fp_string_replace_first(ptr(), find, replace, start);
		}
		Dynamic replace(const string_view find, const string_view replace, size_t start = 0) const {
			return fp_string_view_replace(to_view(), find, replace, start);
		}
		Dynamic replace(const char* find, const char* replace, size_t start = 0) const {
			return fp_string_replace(ptr(), find, replace, start);
		}

#ifdef FP_OSTREAM_SUPPORT
		friend std::ostream& operator<<(std::ostream& s, const string_crtp_common& str) {
			return s << str.derived()->data();
		}
#endif
#if __cpp_lib_format >= 201907L
		friend std::formatter<string_crtp_common, char>;
#endif

	protected:
		inline Derived* derived() { return (Derived*)this; }
		inline const Derived* derived() const { return (Derived*)this; }
		inline char*& ptr() { return derived()->ptr(); }
		inline const char* const & ptr() const { return derived()->ptr(); }
	};

	template<typename Derived>
	struct string_crtp : public string_crtp_common<Derived, Derived> {
		Derived& concatenate_inplace(const Derived& o) { fp_string_concatenate_inplace(ptr(), o.data()); return *derived(); }
		Derived& operator+=(const Derived& o) { concatenate_inplace(o); return *derived(); }
		Derived& concatenate_inplace(const string_view o) { fp_string_view_concatenate_inplace(ptr(), o); return *derived(); }
		Derived& operator+=(const string_view o) { concatenate_inplace(o); return *derived(); }

		Derived concatenate(const Derived& o) const { return fp_string_concatenate(ptr(), o.data()); }
		Derived operator+(const Derived& o) const { return concatenate(o); }
		Derived concatenate(const string_view o) const { return fp_string_view_concatenate(this->to_view(), o); }
		Derived operator+(const string_view o) const { return concatenate(o); }

		Derived& append(const char c) { fp_string_append(ptr(), c); return *derived(); }
		Derived& operator+=(const char c) { return append(c); }
		Derived operator+(const char c) { return std::move(this->make_dynamic() += c); }

		Derived replicate(size_t times) { return fp_string_replicate(ptr(), times); }

		Derived& replace_range_inplace(const string_view with, size_t start, size_t range_len) {
			fp_string_replace_range_inplace(&ptr(), with, start, range_len);
			return *derived();
		}
		Derived& replace_range_inplace(const Derived& with, size_t start, size_t range_len) {
			return replace_range_inplace(with.to_view(), start, range_len);
		}

		Derived& replace_first_inplace(const string_view find, const string_view replace, size_t start = 0) {
			fp_string_replace_first_inplace(&ptr(), find, replace, start);
			return *derived();
		}
		Derived& replace_first_inplace(const Derived& find, const Derived& replace, size_t start = 0) {
			return replace_first_inplace(find.to_view(), replace.to_view(), start);
		}

		Derived& replace_inplace(const string_view find, const string_view replace, size_t start = 0) {
			fp_string_replace_inplace(&ptr(), find, replace, start);
			return *derived();
		}
		Derived& replace_inplace(const Derived& find, const Derived& replace, size_t start = 0) {
			return replace_inplace(find.to_view(), replace.to_view(), start);
		}

#if __cpp_lib_format >= 201907L
		Derived c_format(...) const {
			va_list args;
			va_start(args, ptr());
			auto out = fp_string_vformat(ptr(), args);
			va_end(args);
			return {out};
		}

		template<class... Args>
		Derived format(std::format_string<Args...> fmt, Args&&... args) const {
			auto standard = std::format(fmt, std::forward<Args>(args)...);
			if(standard.empty()) return nullptr;
			return fp_string_make_dynamic(standard.c_str());
		}
#else
		Derived format(...) const {
			va_list args;
			va_start(args, ptr());
			auto out = fp_string_vformat(ptr(), args);
			va_end(args);
			return {out};
		}
#endif

	protected:
		using super = string_crtp_common<Derived, Derived>;
		using super::derived;
		using super::ptr;
	};

	struct string: public dynarray<char>, public string_crtp<string> {
		using super = dynarray<char>;
		using super::super;
		using super::operator=;
		using super::ptr;
		string(const char* str): super(str == nullptr || is_fpda(str) ? (char*)str : fp_string_make_dynamic(str)) {}

		using crtp = string_crtp<string>;
		using crtp::free;
		using crtp::free_and_null;
		// using crtp::clone;
		using crtp::size;

		static fp::dynarray<string> make_dynamic(fp::dynarray<string_view> views) {
			auto out = fp::dynarray<string>{nullptr}.reserve(views.size());
			for(auto view: views)
				out.push_back(view.make_dynamic());
			return out;
		}
	};

	namespace wrapped {
		struct string : public wrapped::pointer<char>, public string_crtp_common<wrapped::string, fp::string> {
			using view = string_view;

			using super = wrapped::pointer<char>;
			using super::super;
			using super::operator=;
			using super::ptr;
			string(const char* str): super((char*)str) {}

			using crtp = string_crtp_common<wrapped::string, fp::string>;
			using crtp::free;
			using crtp::free_and_null;
			using crtp::size;
		};
	}


	inline string string_view::make_dynamic() const { return {fp_string_view_make_dynamic(view_())}; }
	inline int string_view::compare(const string_view o) const { return fp_string_view_compare(view_(), o); }
	inline dynarray<uint32_t> string_view::to_codepoints() const { return fp_string_view_to_codepoints(view_()); }
	inline string string_view::replicate(size_t times) const { return {fp_string_view_replicate(view_(), times)}; }
	inline string string_view::format(...) const {
		va_list args;
		va_start(args, view_());
		auto out = fp_string_view_vformat(view_(), args);
		va_end(args);
		return {out};
	}

	inline string string_view::replace_range(const string_view with, size_t start, size_t len) const {
		return {fp_string_view_replace_range(view_(), with, start, len)};
	}
	inline size_t string_view::replace_first(const string_view find, const string_view replace, size_t start /* = 0 */) const {
		return fp_string_view_replace_first(view_(), find, replace, start);
	}
	inline string string_view::replace(const string_view find, const string_view replace, size_t start /* = 0 */) const {
		return {fp_string_view_replace(view_(), find, replace, start)};
	}


	namespace raii {
		struct string: public raii::dynarray<char>, public string_crtp<raii::string> {
			using view = string_view;

			using super = raii::dynarray<char>;
			using super::ptr;

			using crtp = string_crtp<raii::string>;
			using crtp::free;
			using crtp::free_and_null;
			using crtp::size;

			string(): super(nullptr) {}
			string(const char* str): super(str == nullptr || is_fpda(str) ? (char*)str : fp_string_make_dynamic(str)) {}
			string(const fp::dynarray<char>& o): super(std::move(o.clone())) {}
			string(fp::dynarray<char>&& o): super(std::exchange(o.raw, nullptr)) {}
			string(const string& o): super(std::move(o.clone())) {}
			string(string&& o): super(std::exchange(o.raw, nullptr)) {}
			string& operator=(const string& o) { if(super::raw) crtp::free(); super::raw = (char*)o.clone(); return *this;}
			string& operator=(string&& o) { if(super::raw) crtp::free(); super::raw = std::exchange(o.raw, nullptr); return *this; }
			~string() { if(super::raw) crtp::free_and_null(); }

			static fp::dynarray<raii::string> make_dynamic(fp::dynarray<string_view> views) {
				auto out = fp::dynarray<raii::string>{nullptr}.reserve(views.size());
				for(auto view: views)
					out.push_back(view.make_dynamic());
				return out;
			}
		};
	}

#if __cpp_lib_format >= 201907L
	namespace builder {
		struct string: public raii::string {
			using raii = raii::string;
			using raii::raii;

			template<typename T>
			requires(requires{std::formatter<T, char>{};})
			inline string& operator<<(T value) {
				*this += raii::format("{}", value).release();
				return *this;
			}

			inline string& operator<<(const char c) {
				append(c);
				return *this;
			}

			inline string& operator<<(const char* str) {
				concatenate_inplace(str);
				return *this;
			}

			inline string& operator<<(const fp::string& str) {
				concatenate_inplace(str);
				return *this;
			}

			inline string& operator<<(const fp::raii::string& str) {
				concatenate_inplace(str);
				return *this;
			}

			inline string& operator<<(const fp::string_view& view) {
				*this += view;
				return *this;
			}

			inline string& operator<<(const fp_string_view& view) {
				return *this << fp::string_view{view};
			}
		};
	}
#endif
}

#if __cpp_lib_format >= 201907L
namespace std {
	// template<typename Derived, typename Dynamic>
	// struct formatter<fp::string_view, char> : public formatter<std::string_view, char> {
	// 	using super = formatter<std::string_view, char>;
	
	// 	template<class FmtContext>
	// 	FmtContext::iterator format(const fp::string_view str, FmtContext& ctx) const {
	// 		std::string_view view{str.data(), str.size()};
	// 		return super::format(view, ctx);
	// 	}
	// };

	template<typename Derived, typename Dynamic>
	struct formatter<fp::string_crtp_common<Derived, Dynamic>, char> : public formatter<std::string_view, char> {
		using super = formatter<std::string_view, char>;
	
		template<class FmtContext>
		FmtContext::iterator format(const fp::string_crtp_common<Derived, Dynamic> str, FmtContext& ctx) const {
			std::string_view view{str.ptr(), str.size()};
			return super::format(view, ctx);
		}
	};

	template<typename Derived>
	struct formatter<fp::string_crtp<Derived>, char> : public formatter<fp::string_crtp_common<Derived, Derived>, char> {};

	template<>
	struct formatter<fp::string, char> : public formatter<fp::string_crtp<fp::string>, char> {};
	template<>
	struct formatter<fp::raii::string, char> : public formatter<fp::string_crtp<fp::raii::string>, char> {};
}
#endif 