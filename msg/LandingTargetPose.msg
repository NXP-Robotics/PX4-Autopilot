# Relative position of precision land target in navigation (body fixed, north aligned, NED) and inertial (world fixed, north aligned, NED) frames

uint64 timestamp		# time since system start (microseconds)

bool is_static			# Flag indicating whether the landing target is static or moving with respect to the ground

bool rel_pos_valid		# Flag showing whether relative position is valid
bool rel_vel_valid		# Flag showing whether relative velocity is valid

float32 x_rel			# X/north position of target, relative to vehicle (navigation frame) [meters]
float32 y_rel			# Y/east position of target, relative to vehicle (navigation frame) [meters]
float32 z_rel			# Z/down position of target, relative to vehicle (navigation frame) [meters]

float32 vx_rel			# X/north velocity  of target, relative to vehicle (navigation frame) [meters/second]
float32 vy_rel			# Y/east velocity of target, relative to vehicle (navigation frame) [meters/second]

float32 cov_x_rel		# X/north position variance [meters^2]
float32 cov_y_rel		# Y/east position variance [meters^2]

float32 cov_vx_rel		# X/north velocity variance [(meters/second)^2]
float32 cov_vy_rel		# Y/east velocity variance [(meters/second)^2]

bool abs_pos_valid		# Flag showing whether absolute position is valid
float32 x_abs			# X/north position of target, relative to origin (navigation frame) [meters]
float32 y_abs			# Y/east position of target, relative to origin (navigation frame) [meters]
float32 z_abs			# Z/down position of target, relative to origin (navigation frame) [meters]

#Mavlink Extension:
uint8 num			# The ID of the target if multiple targets are present
float32 angle_x			# X-axis angular offset of the target from the center of the image
float32 angle_y			# y-axis angular offset of the target from the center of the image
float32 distance		# Distance to the target from the vehicle
float32 size_x			# Size of target along x-axis
float32 size_y			# Size of target along y-axis
float32[4] q 			# Quaternion of landing target orientation (w, x, y, z order, zero-rotation is 1, 0, 0, 0) in relation to the Drone

uint8 frame			# Coordinate frame used for following fields.
uint8 type					# Type of landing target
uint8 LANDING_TARGET_TYPE_LIGHT_BEACON = 0	# Landing target signaled by light beacon (ex: IR-LOCK)
uint8 LANDING_TARGET_TYPE_RADIO_BEACON = 1	# Landing target signaled by radio beacon (ex: ILS, NDB)
uint8 LANDING_TARGET_TYPE_VISION_FIDUCIAL = 2	# Landing target represented by a fiducial marker (ex: ARTag)
uint8 LANDING_TARGET_TYPE_VISION_OTHER = 3	# Landing target represented by a pre-defined visual shape/feature (ex: X-marker, H-marker, square)
