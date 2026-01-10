#pragma once
#include "library/iocp/actor/system.h"

enum mail_type {
	dummy_tick, session_disconnect, session_receive
};

struct receive_mail : public actor::mail {
	unsigned long long _value;
public:
	receive_mail(unsigned long long value) noexcept
		: mail(mail_type(session_receive)), _value(value) {
	}
};