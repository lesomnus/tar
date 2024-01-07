#include "tar/ustar.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <ios>
#include <iterator>
#include <numeric>
#include <system_error>
#include <utility>

#include "tar/detail/marshal.hpp"

namespace tar {
namespace ustar {

header header::from(tar::header const& h) {
	header ret{
	    .typeflag = h.type,
	};

	auto const [name, prefix] = detail::split_ustar_path(h.path);

	using detail::marshal;
	marshal(name, ret.name);
	marshal(std::to_underlying(h.permissions), ret.mode);
	marshal(h.uid, ret.uid);
	marshal(h.gid, ret.gid);
	marshal(h.size, ret.size);
	marshal(std::chrono::duration_cast<std::chrono::seconds>(h.last_write_time.time_since_epoch()).count(), ret.mtime);
	marshal(h.link.string(), ret.linkname);

	marshal(h.user_name, ret.uname);
	marshal(h.group_name, ret.gname);
	marshal(0, ret.devmajor);
	marshal(0, ret.devminor);
	marshal(prefix, ret.prefix);

	return ret;
}

header::operator tar::header() {
	tar::header h{
	    .type = this->typeflag,
	};

	using detail::unmarshal;
	{
		std::string v;
		std::string p;
		unmarshal(this->prefix, v);
		unmarshal(this->name, p);
		h.path = v.empty() ? std::filesystem::path(p) : (std::filesystem::path(v) / p);

		unmarshal(this->linkname, p);
		h.link = p;
	}
	{
		std::size_t v;
		unmarshal(this->mode, v);
		h.permissions = static_cast<std::filesystem::perms>(v);
	}
	unmarshal(this->uid, h.uid);
	unmarshal(this->gid, h.gid);
	unmarshal(this->size, h.size);
	{
		std::size_t v;
		unmarshal(this->mtime, v);
		h.last_write_time = std::filesystem::file_time_type(std::chrono::seconds(v));
	}

	unmarshal(this->uname, h.user_name);
	unmarshal(this->gname, h.group_name);
	unmarshal(this->devmajor, h.device_number_major);
	unmarshal(this->devminor, h.device_number_minor);

	return h;
}

istream::istream(std::streambuf* buf)
    : tar::istream()
    , buf_(buf) {
	this->init(&this->buf_);
	this->header_next_ = this->tellg();
}

istream::~istream() { }

istream& istream::next(header& h) {
	auto const body_begin = this->header_next_ + static_cast<off_type>(sizeof(header));
	this->buf_.reset(this->header_next_, body_begin);
	this->clear();
	this->read(reinterpret_cast<char*>(&h), sizeof(h));
	if(!this->operator bool()) [[unlikely]] {
		return *this;
	}

	std::size_t size;
	detail::unmarshal(h.size, size);

	this->buf_.reset(body_begin, body_begin + static_cast<off_type>(size));
	this->header_next_ = body_begin + static_cast<off_type>((size / BlockSize + 1) * BlockSize);

	return *this;
}

ostream::~ostream() {
	this->seal_();
	std::fill_n(std::ostream_iterator<char>(*this), BlockSize * 2, 0);
}

ostream& ostream::next(header const& h) {
	if(this->header_pos_ != -1) [[likely]] {
		this->seal_();
	}

	this->header_cur_ = h;
	this->header_pos_ = this->tellp();  // Remember where the header is to update some fields (size, chksum) later.

	this->write(reinterpret_cast<char const*>(&this->header_cur_), sizeof(header));
	this->seekp(this->header_pos_ + static_cast<off_type>(sizeof(header)));

	return *this;
}

void ostream::seal_() {
	auto const cur = this->tellp();

	if(auto const size = cur - (this->header_pos_ + static_cast<off_type>(sizeof(header))); size < 0) {
		throw std::system_error(std::make_error_code(std::errc::invalid_seek));
	} else {
		detail::marshal(size, this->header_cur_.size);

		auto const* begin = reinterpret_cast<std::uint8_t*>(&this->header_cur_);
		auto const  sum   = std::accumulate(begin, begin + sizeof(header), 0);
		detail::marshal(sum, this->header_cur_.chksum, 6);
		this->header_cur_.chksum[6] = '\0';
		this->header_cur_.chksum[7] = ' ';
	}

	this->seekp(this->header_pos_ + static_cast<off_type>(offsetof(header, size)));
	this->write(this->header_cur_.size.data(), this->header_cur_.size.size() - 1);

	this->seekp(this->header_pos_ + static_cast<off_type>(offsetof(header, chksum)));
	this->write(this->header_cur_.chksum.data(), this->header_cur_.chksum.size() - 1);

	auto const pad_size = BlockSize - (cur % BlockSize);
	this->seekp(cur);
	std::fill_n(std::ostream_iterator<char>(*this), pad_size, 0);
}

}  // namespace ustar
}  // namespace tar
