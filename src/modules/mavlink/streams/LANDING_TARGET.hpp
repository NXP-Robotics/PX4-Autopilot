/****************************************************************************
 *
 *   Copyright (c) 2021 PX4 Development Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

#ifndef LANDING_TARGET_HPP
#define LANDING_TARGET_HPP

#include <uORB/topics/landing_target_pose.h>

class MavlinkStreamLandingTarget : public MavlinkStream
{
public:
	static MavlinkStream *new_instance(Mavlink *mavlink) { return new MavlinkStreamLandingTarget(mavlink); }

	static constexpr const char *get_name_static() { return "LANDING_TARGET"; }
	static constexpr uint16_t get_id_static() { return MAVLINK_MSG_ID_LANDING_TARGET; }

	const char *get_name() const override { return get_name_static(); }
	uint16_t get_id() override { return get_id_static(); }

	unsigned get_size() override
	{
		return _landing_target_sub.advertised() ? MAVLINK_MSG_ID_LANDING_TARGET_LEN + MAVLINK_NUM_NON_PAYLOAD_BYTES : 0;
	}

private:
	explicit MavlinkStreamLandingTarget(Mavlink *mavlink) : MavlinkStream(mavlink) {}

	uORB::Subscription _landing_target_sub{ORB_ID(landing_target_pose)};

	bool send() override
	{
		landing_target_pose_s target;

		if (_landing_target_sub.update(&target)) {

			mavlink_landing_target_t msg{};

			msg.time_usec = target.timestamp;
			msg.frame = target.frame;
			msg.target_num = target.num;
			msg.type = target.type;
			msg.angle_x = target.angle_x;
			msg.angle_y = target.angle_y;
			msg.size_x = target.size_x;
			msg.size_y = target.size_y;
			msg.distance = target.distance;
			msg.x = target.x_rel;
			msg.y = target.y_rel;
			msg.z = target.z_rel;
			msg.q[0] = target.q[0];
			msg.q[1] = target.q[1];
			msg.q[2] = target.q[2];
			msg.q[3] = target.q[3];
			msg.position_valid = target.rel_pos_valid;


			mavlink_msg_landing_target_send_struct(_mavlink->get_channel(), &msg);
			return true;
		}

		return false;
	}
};

#endif // LANDING_TARGET_HPP
