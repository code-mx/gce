﻿///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#include <gce/actor/slice.hpp>
#include <gce/actor/mixin.hpp>
#include <gce/actor/context.hpp>
#include <gce/actor/detail/cache_pool.hpp>
#include <gce/actor/detail/mailbox.hpp>
#include <gce/actor/message.hpp>
#include <gce/detail/scope.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/bind.hpp>
#include <boost/variant/get.hpp>

namespace gce
{
///----------------------------------------------------------------------------
slice::slice(mixin* sire)
  : basic_actor(
      sire->get_context().get_attributes().max_cache_match_size_,
      sire->get_context().get_timestamp()
      )
  , detail::ref_count_st(boost::bind(&slice::free, this))
  , sire_(sire)
{
  owner_ = sire_->select_cache_pool();
}
///----------------------------------------------------------------------------
slice::~slice()
{
}
///----------------------------------------------------------------------------
aid_t slice::recv(message& msg, match_list_t const& match_list)
{
  aid_t sender;
  detail::recv_t rcv;

  mixin::move_pack(this, mb_, pack_que_, owner_, sire_);
  if (!mb_.pop(rcv, msg, match_list))
  {
    return sender;
  }

  if (aid_t* aid = boost::get<aid_t>(&rcv))
  {
    sender = *aid;
  }
  else if (detail::request_t* req = boost::get<detail::request_t>(&rcv))
  {
    sender = req->get_aid();
    msg.req_ = *req;
  }
  else if (detail::exit_t* ex = boost::get<detail::exit_t>(&rcv))
  {
    sender = ex->get_aid();
  }

  return sender;
}
///----------------------------------------------------------------------------
void slice::send(aid_t recver, message const& m)
{
  detail::pack* pk = basic_actor::alloc_pack(owner_);
  pk->tag_ = get_aid();
  pk->recver_ = recver;
  pk->msg_ = m;

  recver.get_actor_ptr(
    owner_->get_ctxid(),
    owner_->get_context().get_timestamp()
    )->on_recv(pk);
}
///----------------------------------------------------------------------------
void slice::relay(aid_t des, message& m)
{
  detail::pack* pk = base_type::alloc_pack(owner_);
  if (m.req_.valid())
  {
    pk->tag_ = m.req_;
    m.req_ = detail::request_t();
  }
  else
  {
    pk->tag_ = get_aid();
  }
  pk->recver_ = des;
  pk->msg_ = m;

  des.get_actor_ptr(
    owner_->get_ctxid(),
    owner_->get_context().get_timestamp()
    )->on_recv(pk);
}
///----------------------------------------------------------------------------
response_t slice::request(aid_t target, message const& m)
{
  aid_t sender = get_aid();
  response_t res(base_type::new_request(), sender);
  detail::request_t req(res.get_id(), sender);

  detail::pack* pk = basic_actor::alloc_pack(owner_);
  pk->tag_ = req;
  pk->recver_ = target;
  pk->msg_ = m;

  target.get_actor_ptr(
    owner_->get_ctxid(),
    owner_->get_context().get_timestamp()
    )->on_recv(pk);
  return res;
}
///----------------------------------------------------------------------------
void slice::reply(aid_t recver, message const& m)
{
  basic_actor* a =
    recver.get_actor_ptr(
      owner_->get_ctxid(),
      owner_->get_context().get_timestamp()
      );
  detail::request_t req;
  detail::pack* pk = basic_actor::alloc_pack(owner_);
  if (mb_.pop(recver, req))
  {
    response_t res(req.get_id(), get_aid());
    pk->tag_ = res;
    pk->recver_ = recver;
    pk->msg_ = m;
  }
  else
  {
    pk->tag_ = get_aid();
    pk->recver_ = recver;
    pk->msg_ = m;
  }
  a->on_recv(pk);
}
///----------------------------------------------------------------------------
aid_t slice::recv(response_t res, message& msg)
{
  aid_t sender;

  mixin::move_pack(this, mb_, pack_que_, owner_, sire_);
  if (!mb_.pop(res, msg))
  {
    return sender;
  }

  sender = res.get_aid();
  return sender;
}
///----------------------------------------------------------------------------
void slice::link(aid_t target)
{
  basic_actor::link(detail::link_t(linked, target), owner_);
}
///----------------------------------------------------------------------------
void slice::monitor(aid_t target)
{
  basic_actor::link(detail::link_t(monitored, target), owner_);
}
///----------------------------------------------------------------------------
void slice::set_ctxid(ctxid_t ctxid)
{
  sire_->set_ctxid(ctxid);
}
///----------------------------------------------------------------------------
void slice::init(aid_t link_tgt)
{
  base_type::update_aid(owner_->get_ctxid());
  if (link_tgt)
  {
    base_type::add_link(link_tgt);
  }
}
///----------------------------------------------------------------------------
void slice::on_free()
{
  base_type::on_free();
}
///----------------------------------------------------------------------------
void slice::on_recv(detail::pack* pk)
{
  pack_que_.push(pk);
}
///----------------------------------------------------------------------------
void slice::free()
{
  base_type::send_exit(exit_normal, "exit normal", owner_);
  base_type::update_aid(owner_->get_ctxid());
  sire_->free_slice(this);
}
///----------------------------------------------------------------------------
}

