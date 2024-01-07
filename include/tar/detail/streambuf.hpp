#pragma once

#include <ios>
#include <streambuf>
#include <string>

namespace tar {
namespace detail {

template<class CharT, class Traits = std::char_traits<CharT>>
class basic_streambuf_wrapper: public std::basic_streambuf<CharT, Traits> {
   public:
	using streambuf_type = std::basic_streambuf<CharT, Traits>;
	using typename streambuf_type::char_type;
	using typename streambuf_type::traits_type;
	using typename streambuf_type::int_type;
	using typename streambuf_type::pos_type;
	using typename streambuf_type::off_type;

	basic_streambuf_wrapper(std::basic_streambuf<CharT, Traits>* base)
	    : base_(base) { }

	streambuf_type const* base() const {
		return this->base_;
	}

	streambuf_type* base() {
		return this->base_;
	}

   protected:
	void imbue(std::locale const& loc) override {
		this->base_->pubimbue(loc);
	}

	streambuf_type* setbuf(char_type* s, std::streamsize n) override {
		return this->base_->pubsetbuf(s, n);
	}

	pos_type seekoff(off_type off, std::ios_base::seekdir dir, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out) override {
		return this->base_->pubseekoff(off, dir, which);
	}

	pos_type seekpos(pos_type pos, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out) override {
		return this->base_->pubseekpos(pos, which);
	}

	int sync() override {
		return this->base_->pubsync();
	}

	int_type underflow() override {
		return this->base_->sgetc();
	}

	int_type uflow() override {
		return this->base_->sbumpc();
	}

	std::streamsize xsgetn(char_type* s, std::streamsize count) override {
		return this->base_->sgetn(s, count);
	}

	int_type overflow(int_type ch = Traits::eof()) override {
		return this->base_->sputc(ch);
	}

	std::streamsize xsputn(const char_type* s, std::streamsize count) override {
		return this->base_->sputn(s, count);
	}

	streambuf_type* base_;
};

using streambuf_wrapper = basic_streambuf_wrapper<std::streambuf::char_type>;

template<class CharT, class Traits = std::char_traits<CharT>>
class basic_scoped_streambuf: public basic_streambuf_wrapper<CharT, Traits> {
   public:
	using streambuf_type = std::basic_streambuf<CharT, Traits>;
	using typename streambuf_type::char_type;
	using typename streambuf_type::traits_type;
	using typename streambuf_type::int_type;
	using typename streambuf_type::pos_type;
	using typename streambuf_type::off_type;

	basic_scoped_streambuf(std::basic_streambuf<CharT, Traits>* base)
	    : basic_streambuf_wrapper<CharT, Traits>(base) { }

	void reset(pos_type begin, pos_type end) {
		this->begin_ = begin;
		this->end_   = end;

		this->base_->pubseekpos(this->begin_, std::ios_base::in);
	}

	int_type underflow() override {
		if(this->end_ <= this->cur_()) [[unlikely]] {
			return traits_type::eof();
		}

		return this->base_->sgetc();
	}

	std::streamsize xsgetn(char_type* s, std::streamsize count) override {
		if(off_type const remain = this->end_ - this->cur_(); remain < count) [[unlikely]] {
			count = remain;
		}
		return this->base_->sgetn(s, count);
	}

   protected:
	pos_type cur_() {
		return this->base_->pubseekoff(0, std::ios_base::cur, std::ios_base::in);
	}

   private:
	pos_type begin_;
	pos_type end_;
};

using scoped_streambuf = basic_scoped_streambuf<std::streambuf::char_type>;

}  // namespace detail
}  // namespace tar
