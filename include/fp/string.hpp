#pragma once

#include "pointer.hpp"
#include "string.h"
#include "dynarray.hpp"
#include <compare>

namespace fp {

	struct string_view: public view<char> {
		struct string make_dynamic() const;
		int compare(string_view o) const;
		struct string format(...) const;

		std::strong_ordering operator<=>(string_view o) const {
			auto res = compare(o);
			if(res < 0) return std::strong_ordering::less;
			if(res > 0) return std::strong_ordering::greater;
			return std::strong_ordering::equal;
		}
		bool operator==(string_view o) const { return this->operator<=>(o) == std::strong_ordering::equal; }
	};

	template<typename Derived, typename Dynamic>
	struct string_crtp_common {
		static Dynamic make_dynamic(char* string) { return fp_string_make_dynamic(string); }
		Dynamic make_dynamic() { return make_dynamic(ptr()); }
		void free() { fp_string_free(ptr()); }
		void free_and_null() { fp_string_free_and_null(ptr()); }

		string_view to_view() { return fp_string_to_view(ptr()); }
		const string_view to_view() const { return {fp_string_to_view_const(ptr())}; }

		size_t length() const { return fp_string_length(ptr()); }
		size_t size() const { return length(); }
		int compare(char* o) const { return fp_string_compare(ptr(), o); }
		int compare(string_view o) const { return fp_string_view_compare(to_view(), o); }
		std::strong_ordering operator<=>(string_view o) const {
			auto res = compare(o);
			if(res < 0) return std::strong_ordering::less;
			if(res > 0) return std::strong_ordering::greater;
			return std::strong_ordering::equal;
		}
		std::strong_ordering operator<=>(char* o) const {
			auto res = compare(o);
			if(res < 0) return std::strong_ordering::less;
			if(res > 0) return std::strong_ordering::greater;
			return std::strong_ordering::equal;
		}
		bool operator==(string_view o) const { return this->operator<=>(o) == std::strong_ordering::equal; }
		bool operator==(char* o) const { return this->operator<=>(o) == std::strong_ordering::equal; }

	protected:
		inline Derived* derived() { return (Derived*)this; }
		inline const Derived* derived() const { return (Derived*)this; }
		inline char*& ptr() { return derived()->ptr(); }
		inline const char* const & ptr() const { return derived()->ptr(); }
	};

	template<typename Derived>
	struct string_crtp : public string_crtp_common<Derived, Derived> {
		Derived& concatenate_inplace(char* o) { return fp_string_concatenate_inplace(ptr(), o); }
		Derived& operator+=(char* o) { return concatenate_inplace(o); }
		Derived& concatenate_inplace(string_view o) { return fp_string_view_concatenate_inplace(ptr(), o); }
		Derived& operator+=(string_view o) { return concatenate_inplace(o); }

		Derived concatenate(char* o) const { return fp_string_concatenate(ptr(), o); }
		Derived& operator+(char* o) const { return concatenate(o); }
		Derived concatenate(string_view o) const { return fp_string_view_concatenate(this->to_view(), o); }
		Derived& operator+(string_view o) const { return concatenate(o); }

		Derived& append(char c) { return fp_string_append(ptr(), c); }
		Derived& operator+=(char c) { return append(c); }

		Derived format(...) const {
			va_list args;
			va_start(args, ptr());
			auto out = fp_string_vformat(ptr(), args);
			va_end(args);
			return {out};
		}

	protected:
		using super = string_crtp_common<Derived, Derived>;
		using super::derived;
		using super::ptr;
	};

	struct string: public dynarray<char>, public string_crtp<string> {
		// using super = pointer<char>;
		// using super::super;
		// using super::operator=;
		// using super::ptr;

		using crtp = string_crtp<string>;
		using crtp::free;
		using crtp::free_and_null;
		// using crtp::clone;
	};

	namespace wrapped {
		struct string : public wrapped::pointer<char>, public string_crtp_common<string, fp::string> {
			using crtp = string_crtp_common<string, fp::string>;
			using crtp::free;
			using crtp::free_and_null;
		};
	}


	inline string string_view::make_dynamic() const { return {fp_string_view_make_dynamic(view_())}; }
	inline int string_view::compare(string_view o) const { return fp_string_view_compare(view_(), o); }
	inline string string_view::format(...) const {
		va_list args;
		va_start(args, view_());
		auto out = fp_string_view_vformat(view_(), args);
		va_end(args);
		return {out};
	}


	namespace raii {
		struct string: public raii::dynarray<char>, public string_crtp<raii::string> {
			using super = raii::dynarray<char>;
			using super::ptr;

			using crtp = string_crtp<raii::string>;

			string(): super(nullptr) {}
			string(char* ptr): super(ptr) {}
			string(const fp::dynarray<char>& o): super(std::move(o.clone())) {}
			string(fp::dynarray<char>&& o): super(std::exchange(o.raw, nullptr)) {}
			string(const string& o): super(std::move(o.clone())) {}
			string(string&& o): super(std::exchange(o.raw, nullptr)) {}
			string& operator=(const string& o) { if(super::raw) crtp::free(); super::raw = o.clone(); return *this;}
			string& operator=(string&& o) { if(super::raw) crtp::free(); super::raw = std::exchange(o.raw, nullptr); return *this; }
			~string() { if(super::raw) crtp::free_and_null(); }
		};
	}
}