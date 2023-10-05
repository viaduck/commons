/*
 * Copyright (C) 2023 The ViaDuck Project
 *
 * This file is part of Commons.
 *
 * Commons is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Commons is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Commons.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef COMMONS_TYPES_H
#define COMMONS_TYPES_H

#include <flatbuffers/network/data/ReqDataHeader.h>
#include <flatbuffers/network/data/ResDataHeader.h>
#include <flatbuffers/network/data/count/ReqMessageCount.h>
#include <flatbuffers/network/data/count/ResMessageCount.h>
#include <flatbuffers/network/data/get/ReqGetMessages.h>
#include <flatbuffers/network/data/get/ResGetMessages.h>
#include <flatbuffers/network/data/get/ResMessage.h>
#include <flatbuffers/network/data/send/ReqSendMessage.h>

#include <flatbuffers/network/push/ReqPushHeader.h>
#include <flatbuffers/network/push/ResPushHeader.h>
#include <flatbuffers/network/push/RepHeartbeat.h>
#include <flatbuffers/network/push/RepPostbox.h>
#include <flatbuffers/network/push/notify/ResNotify.h>
#include <flatbuffers/network/push/subscribe/ReqSubUnsub.h>

#endif //COMMONS_TYPES_H
