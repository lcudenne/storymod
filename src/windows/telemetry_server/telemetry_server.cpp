
/**************************************************************************************
 * STORYMOD
 *
 * Website: https://sites.google.com/site/storymodsite
 * GitHub: https://github.com/lcudenne/storymod
 * Author: Loic Cudennec <loic@cudennec.fr>
 *         SCS Software
 *
 * This program is a telemetry UDP stream client for Euro Truck Simulator 2 and
 * American Truck Simulator. It waits for a storymod server connection and streams
 * the truck state and position.
 *
 **************************************************************************************
 *
 * SCS SDK
 * Copyright (C) 2013 SCS Software
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 **************************************************************************************/


/**
 * @brief Logs calculated world space position of the head to demonstrate
 * combination of telemetry channels and configuration data.
 */

// Windows stuff.

#define WINVER 0x0500
#define _WIN32_WINNT 0x0500
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <math.h>

// SDK

#include "scssdk_telemetry.h"
#include "eurotrucks2/scssdk_eut2.h"
#include "eurotrucks2/scssdk_telemetry_eut2.h"

#include "telemetry_server_network.h"

#define UNUSED(x)

/**
 * @brief Logging support.
 */
FILE *log_file = NULL;

/**
 * @brief Tracking of paused state of the game.
 */
bool output_paused = true;

/**
 * @brief Combined telemetry data.
 */
struct telemetry_state_t
{
	// Configuration.

	/**
	 * @brief Position of the cabin joint in vehicle space.
	 */
	scs_value_fvector_t	cabin_position;

	/**
	 * @brief Base position of the head in the cabin.
	 */
	scs_value_fvector_t	head_position;

	// Channels.

	/**
	 * @brief World space position & orientation of the truck.
	 */
	scs_value_dplacement_t	truck_placement;

	/**
	 * @brief Offset of the cabin from the default position.
	 */
	scs_value_fplacement_t	cabin_offset;

	/**
	 * @brief Offset of the head from its default position.
	 */
	scs_value_fplacement_t	head_offset;

	/**
	* @brief Left blinker.
	*/
	scs_value_bool_t lblinker;

	/**
	* @brief Right blinker.
	*/
	scs_value_bool_t rblinker;

	/**
	* @brief Trailer connected.
	*/
	scs_value_bool_t trailer_connected;


} telemetry;

/**
 * @brief Function writting message to the game internal log.
 */
scs_log_t game_log = NULL;

// Management of the log file.

bool init_log(void)
{
	if (log_file) {
		return true;
	}
	log_file = fopen("telemetry_position.log", "wt");
	if (! log_file) {
		return false;
	}
	fprintf(log_file, "Log opened\n");
	return true;
}

void finish_log(void)
{
	if (! log_file) {
		return;
	}
	fprintf(log_file, "Log ended\n");
	fclose(log_file);
	log_file = NULL;
}

void log_print(const char *const text, ...)
{
	if (! log_file) {
		return;
	}
	va_list args;
	va_start(args, text);
	vfprintf(log_file, text, args);
	va_end(args);
}

void log_line(const char *const text, ...)
{
	if (! log_file) {
		return;
	}
	va_list args;
	va_start(args, text);
	vfprintf(log_file, text, args);
	fprintf(log_file, "\n");
	va_end(args);
}

/**
 * @brief Adds two float vectors.
 */
scs_value_fvector_t  add(const scs_value_fvector_t &first, const scs_value_fvector_t &second)
{
	scs_value_fvector_t result;
	result.x = first.x + second.x;
	result.y = first.y + second.y;
	result.z = first.z + second.z;
	return result;
}

/**
 * @brief Adds float vector to double vector.
 */
scs_value_dvector_t  add(const scs_value_dvector_t &first, const scs_value_fvector_t &second)
{
	scs_value_dvector_t result;
	result.x = first.x + second.x;
	result.y = first.y + second.y;
	result.z = first.z + second.z;
	return result;
}

/**
 * @brief Rotates specified vector by specified orientation.
 */
scs_value_fvector_t rotate(const scs_value_euler_t &orientation, const scs_value_fvector_t &vector)
{
	const float heading_radians = orientation.heading * 6.2831853071795864769252867665590058f;
	const float pitch_radians = orientation.pitch * 6.2831853071795864769252867665590058f;
	const float roll_radians = orientation.roll * 6.2831853071795864769252867665590058f;

	const float cos_heading = cosf(heading_radians);
	const float sin_heading = sinf(heading_radians);
	const float cos_pitch = cosf(pitch_radians);
	const float sin_pitch = sinf(pitch_radians);
	const float cos_roll = cosf(roll_radians);
	const float sin_roll = sinf(roll_radians);

	// Roll around Z axis.

	const float post_roll_x = vector.x * cos_roll - vector.y * sin_roll;
	const float post_roll_y = vector.x * sin_roll + vector.y * cos_roll;
	const float post_roll_z = vector.z;

	// Pitch around X axis.

	const float post_pitch_x = post_roll_x;
	const float post_pitch_y = post_roll_y * cos_pitch - post_roll_z * sin_pitch;
	const float post_pitch_z = post_roll_y * sin_pitch + post_roll_z * cos_pitch;

	// Heading around Y axis.

	scs_value_fvector_t result;
	result.x = post_pitch_x * cos_heading + post_pitch_z * sin_heading;
	result.y = post_pitch_y;
	result.z = -post_pitch_x * sin_heading + post_pitch_z * cos_heading;
	return result;
}

// Handling of individual events.

SCSAPI_VOID telemetry_frame_end(const scs_event_t UNUSED(event), const void *const UNUSED(event_info), const scs_context_t UNUSED(context))
{
	if (output_paused) {
		return;
	}

	// Calculate the position. Note that the value differs slightly from the values used by the
	// game for rendering. This code evaluates the value using data directly corresponding to
	// simulation steps instead of interpolating between two neighbour simulated positions like
	// the game does. It also calculates the value for each simulation frame instead of once per
	// rendering frame like the game does.

	const scs_value_fvector_t head_position_in_cabin_space = add(telemetry.head_position, telemetry.head_offset.position);
	const scs_value_fvector_t head_position_in_vehicle_space = add(add(telemetry.cabin_position, telemetry.cabin_offset.position), rotate(telemetry.cabin_offset.orientation, head_position_in_cabin_space));
	const scs_value_dvector_t head_position_in_world_space = add(telemetry.truck_placement.position, rotate(telemetry.truck_placement.orientation, head_position_in_vehicle_space));

	sendPositionToUDP(head_position_in_world_space.x, head_position_in_world_space.y, head_position_in_world_space.z);

	// log_line("%f;%f;%f", head_position_in_world_space.x, head_position_in_world_space.y, head_position_in_world_space.z);
}

SCSAPI_VOID telemetry_pause(const scs_event_t event, const void *const UNUSED(event_info), const scs_context_t UNUSED(context))
{
	output_paused = (event == SCS_TELEMETRY_EVENT_paused);
}


/**
 * @brief Finds attribute with specified name in the configuration structure.
 *
 * Returns NULL if the attribute was not found or if it is not of the expected type.
 */
const scs_named_value_t *find_attribute(const scs_telemetry_configuration_t &configuration, const char *const name, const scs_u32_t index, const scs_value_type_t expected_type)
{
	for (const scs_named_value_t *current = configuration.attributes; current->name; ++current) {
		if ((current->index != index) || (strcmp(current->name, name) != 0)) {
			continue;
		}
		if (current->value.type == expected_type) {
			return current;
		}
		log_line("ERROR: Attribute %s has unexpected type %u", name, static_cast<unsigned>(current->value.type));
		break;
	}
	return NULL;
}


SCSAPI_VOID telemetry_configuration(const scs_event_t event, const void *const event_info, const scs_context_t UNUSED(context))
{
	const struct scs_telemetry_configuration_t *const info = static_cast<const scs_telemetry_configuration_t *>(event_info);

	// We currently only care for the truck telemetry info.

	//if (strcmp(info->id, SCS_TELEMETRY_CONFIG_truck) != 0) {
	//	return;
	//}

	// Extract the value we are interested in.

	const scs_named_value_t *const cabin_position = find_attribute(*info, SCS_TELEMETRY_CONFIG_ATTRIBUTE_cabin_position, SCS_U32_NIL, SCS_VALUE_TYPE_fvector);
	if (cabin_position) {
		telemetry.cabin_position = cabin_position->value.value_fvector;
	}
	else {
		// Vehicle without separate cabin.

		telemetry.cabin_position.x = telemetry.cabin_position.y = telemetry.cabin_position.z = 0.0f;
	}

	const scs_named_value_t *const head_position = find_attribute(*info, SCS_TELEMETRY_CONFIG_ATTRIBUTE_head_position, SCS_U32_NIL, SCS_VALUE_TYPE_fvector);
	if (head_position) {
		telemetry.head_position = head_position->value.value_fvector;
	}

	const scs_named_value_t *const cargo_id = find_attribute(*info, SCS_TELEMETRY_CONFIG_ATTRIBUTE_cargo_id, SCS_U32_NIL, SCS_VALUE_TYPE_string);
	const scs_named_value_t *const cargo = find_attribute(*info, SCS_TELEMETRY_CONFIG_ATTRIBUTE_cargo, SCS_U32_NIL, SCS_VALUE_TYPE_string);
	if (cargo_id && cargo) {
		char * cargostr = (char *) malloc(sizeof(char) * (strlen(cargo_id->value.value_string.value) + strlen(cargo->value.value_string.value + 2)));
		assert(cargostr);
		sprintf(cargostr, "%s %s", cargo_id->value.value_string.value, cargo->value.value_string.value);
		sendConfigToUDP(DATAGRAM_TYPE_CARGO, cargostr);
		free(cargostr);
	}


}

// Handling of individual channels.

SCSAPI_VOID telemetry_store_fplacement(const scs_string_t name, const scs_u32_t index, const scs_value_t *const value, const scs_context_t context)
{
	assert(context);
	assert(value);
	assert(value->type == SCS_VALUE_TYPE_fplacement);
	scs_value_fplacement_t *const placement = static_cast<scs_value_fplacement_t *>(context);
	*placement = value->value_fplacement;
}

SCSAPI_VOID telemetry_store_dplacement(const scs_string_t name, const scs_u32_t index, const scs_value_t *const value, const scs_context_t context)
{
	assert(context);
	assert(value);
	assert(value->type == SCS_VALUE_TYPE_dplacement);
	scs_value_dplacement_t *const placement = static_cast<scs_value_dplacement_t *>(context);
	*placement = value->value_dplacement;
}

SCSAPI_VOID telemetry_store_bool(const scs_string_t name, const scs_u32_t index, const scs_value_t *const value, const scs_context_t context)
{
	assert(context);
	assert(value);
	assert(value->type == SCS_VALUE_TYPE_bool);
	scs_value_bool_t *const property = static_cast<scs_value_bool_t *>(context);
	*property = value->value_bool;

	if (telemetry.lblinker.value) {
		sendConfigToUDP(DATAGRAM_TYPE_LBLINKER, "1");
	} else {
		sendConfigToUDP(DATAGRAM_TYPE_LBLINKER, "0");
	}
	if (telemetry.rblinker.value) {
		sendConfigToUDP(DATAGRAM_TYPE_RBLINKER, "1");
	}
	else {
		sendConfigToUDP(DATAGRAM_TYPE_RBLINKER, "0");
	}
	if (telemetry.trailer_connected.value) {
		sendConfigToUDP(DATAGRAM_TYPE_TRAILER_CONNECTED, "1");
	}
	else {
		sendConfigToUDP(DATAGRAM_TYPE_TRAILER_CONNECTED, "0");
	}

}


/**
 * @brief Telemetry API initialization function.
 *
 * See scssdk_telemetry.h
 */
SCSAPI_RESULT scs_telemetry_init(const scs_u32_t version, const scs_telemetry_init_params_t *const params)
{
	// We currently support only one version.

	if (version != SCS_TELEMETRY_VERSION_1_00) {
		return SCS_RESULT_unsupported;
	}

	const scs_telemetry_init_params_v100_t *const version_params = static_cast<const scs_telemetry_init_params_v100_t *>(params);
	if (! init_log()) {
		version_params->common.log(SCS_LOG_TYPE_error, "Unable to initialize the log file");
		return SCS_RESULT_generic_error;
	}

	// Check application version. Note that this example uses fairly basic channels which are likely to be supported
	// by any future SCS trucking game however more advanced application might want to at least warn the user if there
	// is game or version they do not support.

	log_line("Game '%s' %u.%u", version_params->common.game_id, SCS_GET_MAJOR_VERSION(version_params->common.game_version), SCS_GET_MINOR_VERSION(version_params->common.game_version));

	if (strcmp(version_params->common.game_id, SCS_GAME_ID_EUT2) != 0) {
		log_line("WARNING: Unsupported game, some features or values might behave incorrectly");
	}
	else {

		// Bellow the minimum version there might be some missing feature (only minor change) or
		// incompatible values (major change).

		const scs_u32_t MINIMAL_VERSION = SCS_TELEMETRY_EUT2_GAME_VERSION_1_00;
		if (version_params->common.game_version < MINIMAL_VERSION) {
			log_line("WARNING: Too old version of the game, some features might behave incorrectly");
		}

		// Future versions are fine as long the major version is not changed.

		const scs_u32_t IMPLEMENTED_VERSION = SCS_TELEMETRY_EUT2_GAME_VERSION_CURRENT;
		if (SCS_GET_MAJOR_VERSION(version_params->common.game_version) > SCS_GET_MAJOR_VERSION(IMPLEMENTED_VERSION)) {
			log_line("WARNING: Too new major version of the game, some features might behave incorrectly");
		}
	}

	// Register for events. Note that failure to register those basic events
	// likely indicates invalid usage of the api or some critical problem. As the
	// example requires all of them, we can not continue if the registration fails.

	const bool events_registered =
		(version_params->register_for_event(SCS_TELEMETRY_EVENT_frame_end, telemetry_frame_end, NULL) == SCS_RESULT_ok) &&
		(version_params->register_for_event(SCS_TELEMETRY_EVENT_paused, telemetry_pause, NULL) == SCS_RESULT_ok) &&
		(version_params->register_for_event(SCS_TELEMETRY_EVENT_started, telemetry_pause, NULL) == SCS_RESULT_ok) &&
		(version_params->register_for_event(SCS_TELEMETRY_EVENT_configuration, telemetry_configuration, NULL) == SCS_RESULT_ok)
	;
	if (! events_registered) {

		// Registrations created by unsuccessfull initialization are
		// cleared automatically so we can simply exit.

		version_params->common.log(SCS_LOG_TYPE_error, "Unable to register event callbacks");
		return SCS_RESULT_generic_error;
	}

	// Register for channels. The channel might be missing if the game does not support
	// it (SCS_RESULT_not_found) or if does not support the requested type
	// (SCS_RESULT_unsupported_type). For purpose of this example we ignore the failues
	// so the unsupported channels will remain at theirs default value.

	version_params->register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_world_placement, SCS_U32_NIL, SCS_VALUE_TYPE_dplacement, SCS_TELEMETRY_CHANNEL_FLAG_none, telemetry_store_dplacement, &telemetry.truck_placement);
	version_params->register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_cabin_offset, SCS_U32_NIL, SCS_VALUE_TYPE_fplacement, SCS_TELEMETRY_CHANNEL_FLAG_none, telemetry_store_fplacement, &telemetry.cabin_offset);
	version_params->register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_head_offset, SCS_U32_NIL, SCS_VALUE_TYPE_fplacement, SCS_TELEMETRY_CHANNEL_FLAG_none, telemetry_store_fplacement, &telemetry.head_offset);
	version_params->register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_lblinker, SCS_U32_NIL, SCS_VALUE_TYPE_bool, SCS_TELEMETRY_CHANNEL_FLAG_none, telemetry_store_bool, &telemetry.lblinker);
	version_params->register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_rblinker, SCS_U32_NIL, SCS_VALUE_TYPE_bool, SCS_TELEMETRY_CHANNEL_FLAG_none, telemetry_store_bool, &telemetry.rblinker);
	version_params->register_for_channel(SCS_TELEMETRY_TRAILER_CHANNEL_connected, SCS_U32_NIL, SCS_VALUE_TYPE_bool, SCS_TELEMETRY_CHANNEL_FLAG_none, telemetry_store_bool, &telemetry.trailer_connected);

	game_log = version_params->common.log;

	// Set the structure with defaults.

	memset(&telemetry, 0, sizeof(telemetry));

	// Initialize network

	initNetwork();

	// Initially the game is paused.

	output_paused = true;
	return SCS_RESULT_ok;
}

/**
 * @brief Telemetry API deinitialization function.
 *
 * See scssdk_telemetry.h
 */
SCSAPI_VOID scs_telemetry_shutdown(void)
{
	// Any cleanup needed. The registrations will be removed automatically
	// so there is no need to do that manually.

	game_log = NULL;
	finish_log();

	// close network

	closeNetwork();


}

// Telemetry api.

BOOL APIENTRY DllMain(
	HMODULE module,
	DWORD  reason_for_call,
	LPVOID reseved
)
{
	if (reason_for_call == DLL_PROCESS_DETACH) {
		finish_log();
	}
	return TRUE;
}