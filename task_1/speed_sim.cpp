#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <cstdlib>
#include <cstring>

using namespace std;

int const DEFAULT_NUM_SENSORS = 1;
int const DEFAULT_SAMPLING = 10;
int const DEFAULT_INTERVAL = 3600;
string const LOG_FILE_NAME = "task1.log";

enum Error_Enum
{
	INVALID_CMD_LINE_ARG,
	INVALID_NUM_SENSORS,
	INVALID_SAMPLING,
	INVALID_DURATION,
	CANNOT_SAVE,
	INTERVAL_LESS_THAN_SAMPLING
};

// delcare variables and assign default values
int num_sensors = DEFAULT_NUM_SENSORS;
int sampling = DEFAULT_SAMPLING;
int interval = DEFAULT_INTERVAL;

ofstream log_file;

void output_error(int error_code)
{
	string error;
	if (error_code == INVALID_CMD_LINE_ARG)
	{
		error = "error 1.1: invalid command line argument";
	}
	else if (error_code == INVALID_NUM_SENSORS)
	{
		error = "error 1.2: invalid number of sensors";
	}
	else if (error_code == INVALID_SAMPLING)
	{
		error = "error 1.3: invalid number of sampling time";
	}
	else if (error_code == INVALID_DURATION)
	{
		error = "error 1.4: invalid duration";
	}
	else if (error_code == CANNOT_SAVE)
	{
		error = "error 1.5: unable to save data";
	}
	else if (error_code == INTERVAL_LESS_THAN_SAMPLING)
	{
		error = "error 1.6: the simulation duration less than the sampling time";
	}
	cout << error << endl;
	log_file << error << endl;
}

bool assign_value(bool &has_set, int &variable, string value, int error_code)
{
	try
	{
		if (has_set)
		{
			output_error(INVALID_CMD_LINE_ARG);
			return false;
		}
		variable = stoi(value);
		if (variable <= 0)
		{
			output_error(error_code);
			return false;
		}
		has_set = true;
		return true;
	}
	catch (invalid_argument)
	{
		output_error(INVALID_CMD_LINE_ARG);
		output_error(error_code);
		return false;
	}
	catch (out_of_range)
	{
		output_error(error_code);
		return false;
	}
}

bool process_command_line(int argc, char *argv[])
{
	if (argc > 7)
	{
		output_error(INVALID_CMD_LINE_ARG);
		return false;
	}
	bool has_num_sensors_set = false;
	bool has_sampling_set = false;
	bool has_interval_set = false;
	bool result = true;
	for (int i = 1; i < argc - 1; i += 2)
	{
		if (strcmp("-n", argv[i]) == 0)
		{
			result &= assign_value(has_num_sensors_set, num_sensors, argv[i + 1], INVALID_NUM_SENSORS);
		}
		else if (strcmp("-st", argv[i]) == 0)
		{
			result &= assign_value(has_sampling_set, sampling, argv[i + 1], INVALID_SAMPLING);
		}
		else if (strcmp("-si", argv[i]) == 0)
		{
			result &= assign_value(has_interval_set, interval, argv[i + 1], INVALID_DURATION);
		}
		else
		{
			output_error(INVALID_CMD_LINE_ARG);
			return false;
		}
	}
	if (interval < sampling)
	{
		output_error(INTERVAL_LESS_THAN_SAMPLING);
		return false;
	}
	return result;
}

string conver_to_string(time_t time)
{
	tm tm_gmt;
	localtime_s(&tm_gmt, &time);
	ostringstream oss;
	oss << setfill('0') << setw(2) << tm_gmt.tm_hour;
	oss << setfill('0') << setw(2) << tm_gmt.tm_min;
	oss << setfill('0') << setw(2) << tm_gmt.tm_sec;
	return oss.str().insert(4, ":").insert(2, ":");
}

int main(int argc, char *argv[])
{
	log_file.open(LOG_FILE_NAME);
	if (process_command_line(argc, argv))
	{
		time_t now = time(nullptr);
		ofstream new_file;
		new_file.exceptions(ofstream::failbit | ofstream::badbit);
		try
		{
			new_file.open("speed_data_" + to_string(now) + ".csv");
			new_file << "id,time,values";
			srand(now);
			for (int i = 0; i <= interval; i += sampling)
			{
				for (int j = 0; j < num_sensors; j++)
				{
					new_file << endl
							 << j + 1 << "," << conver_to_string(now + i) << "," << rand() % 15001 * 2.0 / 10;
				}
			}
			new_file.close();
		}
		catch (ofstream::failure)
		{
			output_error(CANNOT_SAVE);
		}
	}
	log_file.close();
}