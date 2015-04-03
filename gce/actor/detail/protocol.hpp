///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_DETAIL_PROTOCOL_HPP
#define GCE_ACTOR_DETAIL_PROTOCOL_HPP

#include <gce/actor/config.hpp>

namespace gce
{
namespace detail
{
struct msg_header
{
  msg_header()
    : size_(0)
    , type_(match_nil)
    , tag_offset_(u32_nil)
  {
  }

  uint32_t size_;
  match_t type_;
  uint32_t tag_offset_;
};
}
}

GCE_PACK(gce::detail::msg_header, (v.size_)(v.type_)(v.tag_offset_));

#endif /// GCE_ACTOR_DETAIL_PROTOCOL_HPP
