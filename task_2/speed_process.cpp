#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <ctime>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <algorithm>

using namespace std;

string const DEFAULT_DATA_FILE_NAME = "speed_data.csv";
string const OUTLIER_FILE_NAME = "outlier_data.csv";
string const SUMMARY_FILE_NAME = "data_summary.csv";
string const STATISTICS_FILE_NAME = "data_statistics.csv";
string const SORTED_FILE_NAME = "sorted_data.csv";
string const LOG_FILE_NAME = "task2.log";

enum Error_Enum
{
	FILE_ERROR,
	INVALID_FORMAT,
	MISSING_DATA,
	INVALID_CMD_LINE_ARG,
	CANNOT_SAVE
};

struct record
{
	int id;
	int hour;
	int minute;
	int second;
	float speed;
};

// delcare variables and assign default values
string data_file_name = DEFAULT_DATA_FILE_NAME;
bool need_sort = false;

ifstream data_file;
ofstream log_file;
vector<vector<record>> data_bins;

void output_error(int error_code, int line = -1)
{
	string error;
	if (error_code == FILE_ERROR)
	{
		error = "error 2.1: non existing or not readable data file";
	}
	else if (error_code == INVALID_FORMAT)
	{
		error = "error 2.2: wrong data file format";
	}
	else if (error_code == MISSING_DATA)
	{
		error = "error 2.3: missing data in line " + to_string(line);
	}
	else if (error_code == INVALID_CMD_LINE_ARG)
	{
		error = "error 2.4: invalid command line argument";
	}
	else if (error_code == CANNOT_SAVE)
	{
		error = "error 2.5: unable to save file";
	}
	cout << error << endl;
	log_file << error << endl;
}

bool process_command_line(int argc, char *argv[])
{
	if (argc >= 4)
	{
		output_error(INVALID_CMD_LINE_ARG);
		return false;
	}
	if (argc >= 2)
	{
		data_file_name = argv[1];
		if (argc == 3)
		{
			if (strcmp(argv[2], "-s") == 0)
			{
				need_sort = true;
			}
			else
			{
				output_error(INVALID_CMD_LINE_ARG);
				return false;
			}
		}
	}
	return true;
}

int parse_record(string line, record &result)
{
	if (line.length() == 0)
	{
		return MISSING_DATA;
	}
	string elements[3];

	istringstream iss(line);
	iss.exceptions(ifstream::failbit);
	try
	{
		for (int i = 0; i < 3; i++)
		{
			getline(iss, elements[i], ',');
			if (elements[i].length() == 0)
			{
				return MISSING_DATA;
			}
		}
		result.id = stoi(elements[0]);
		result.speed = stof(elements[2]);

		istringstream iss(elements[1]);
		for (int i = 0; i < 3; i++)
		{
			getline(iss, elements[i], ':');
		}
		result.hour = stoi(elements[0]);
		result.minute = stoi(elements[1]);
		result.second = stoi(elements[2]);
		if (result.id < 0 || result.speed < 0 || result.speed > 3000 || result.hour > 23 || result.hour < 0 || result.minute > 59 || result.minute < 0 || result.second > 59 || result.second < 0)
		{
			return INVALID_FORMAT;
		}
		return 0;
	}
	catch (...)
	{
		return INVALID_FORMAT;
	}
}

bool load_data_and_filter_out_outlier()
{
	string line;
	record new_record;
	int line_num;
	vector<record> outliers;

	getline(data_file, line); // ignore heading line

	line_num = 1;
	while (getline(data_file, line))
	{
		if (parse_record(line, new_record) == INVALID_FORMAT)
		{
			output_error(INVALID_FORMAT);
			return false;
		}
		else if (parse_record(line, new_record) == MISSING_DATA)
		{
			output_error(MISSING_DATA, line_num);
		}
		else if (new_record.speed >= 900 && new_record.speed <= 1600)
		{
			vector<record> empty_bin;
			while (data_bins.size() < new_record.id)
			{
				data_bins.push_back(empty_bin);
			}
			data_bins.at(new_record.id - 1).push_back(new_record);
		}
		else
		{
			outliers.push_back(new_record);
		}
		line_num++;
	}
	ofstream outliers_file(OUTLIER_FILE_NAME);
	outliers_file << "number of outliers : " << outliers.size() << endl;
	outliers_file << "id,time,values";
	for (const record &r : outliers)
	{
		outliers_file << endl
					  << r.id << "," << r.hour << ":" << r.minute << ":" << r.second << "," << r.speed;
	}
	outliers_file.close();
	return true;
}

int time_duration(record start, record end)
{
	int start_time = start.hour * 60 * 60 + start.minute * 60 + start.second;
	int end_time = end.hour * 60 * 60 + end.minute * 60 + end.second;
	return end_time - start_time;
}

void set_simulation_interval(record &result)
{
	record start = data_bins.at(0).at(0);
	record end = data_bins.at(0).at(0);
	for (const vector<record> &bin : data_bins)
	{
		for (const record &r : bin)
		{
			if (time_duration(start, r) < 0)
			{
				start = r;
			}
			if (time_duration(r, end) < 0)
			{
				end = r;
			}
		}
	}
	int duration = time_duration(start, end);
	result.hour = duration / 60 / 60;
	duration -= result.hour * 60 * 60;
	result.minute = duration / 60;
	duration -= result.minute * 60;
	result.second = duration;
}

vector<record> do_summary(vector<record> bin)
{
	record max = bin.at(0);
	record min = bin.at(0);
	record mean = bin.at(0);
	float sum_speed = 0;

	for (const record &r : bin)
	{
		if (r.speed > max.speed)
		{
			max = r;
		}
		if (r.speed < min.speed)
		{
			min = r;
		}
		sum_speed += r.speed;
	}
	mean.speed = sum_speed / bin.size();
	set_simulation_interval(mean);

	vector<record> result;
	result.push_back(max);
	result.push_back(min);
	result.push_back(mean);
	return result;
}

void summary_data()
{
	ofstream summary_file(SUMMARY_FILE_NAME);
	summary_file << "id,parameters,time,values";
	for (const vector<record> &bin : data_bins)
	{
		if (bin.size() == 0)
		{
			continue;
		}
		vector<record> summary = do_summary(bin);
		record r = summary.at(0);
		summary_file << endl
					 << r.id << ",max," << r.hour << ":" << r.minute << ":" << r.second << "," << r.speed;
		r = summary.at(1);
		summary_file << endl
					 << r.id << ",min," << r.hour << ":" << r.minute << ":" << r.second << "," << r.speed;
		r = summary.at(2);
		summary_file << endl
					 << r.id << ",mean," << r.hour << ":" << r.minute << ":" << r.second << "," << r.speed;
	}
	summary_file.close();
}

vector<int> do_statistics(vector<record> bin)
{
	int count_increment = 0;
	int count_decrement = 0;
	for (int i = 0; i < bin.size() - 1; i++)
	{
		int speed_change_per_second = (bin.at(i + 1).speed - bin.at(i).speed) / time_duration(bin.at(i), bin.at(i + 1));
		if (speed_change_per_second > 100)
		{
			count_increment++;
		}
		else if (speed_change_per_second < -100)
		{
			count_decrement++;
		}
	}
	vector<int> result;
	result.push_back(count_increment);
	result.push_back(count_decrement);
	return result;
}

void statistics_data()
{
	ofstream summary_file(STATISTICS_FILE_NAME);
	summary_file << "id,direction,frequency";
	for (const vector<record> &bin : data_bins)
	{
		if (bin.size() == 0)
		{
			continue;
		}
		vector<int> statistics = do_statistics(bin);
		summary_file << endl
					 << bin.at(0).id << ",increment," << statistics.at(0);
		summary_file << endl
					 << bin.at(0).id << ",decrement," << statistics.at(1);
	}
	summary_file.close();
}

bool compare_speed(record r1, record r2)
{
	return (r1.speed < r2.speed);
}

void sort_data()
{
	ofstream sorted_file(SORTED_FILE_NAME);
	vector<record> sorted_set;
	int time_to_sort = 0;
	for (vector<record> bin : data_bins)
	{
		if (bin.size() == 0)
		{
			continue;
		}
		auto begin = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
		sort(bin.begin(), bin.end(), compare_speed);
		auto end = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
		time_to_sort += end - begin;

		for (const record &r : bin)
		{
			sorted_set.push_back(r);
		}
	}

	sorted_file << "sorting duration: " << time_to_sort / 1000.0 << " seconds";
	sorted_file << endl
				<< "id,time,values";
	for (const record &r : sorted_set)
	{
		sorted_file << endl
					<< r.id << "," << r.hour << ":" << r.minute << ":" << r.second << "," << r.speed;
	}
	sorted_file.close();
}

int main(int argc, char *argv[])
{
	log_file.open(LOG_FILE_NAME);
	if (process_command_line(argc, argv))
	{
		data_file.exceptions(ofstream::badbit);
		try
		{
			data_file.open(data_file_name);
			if (!data_file.is_open())
			{
				output_error(FILE_ERROR);
				return -1;
			}
			if (load_data_and_filter_out_outlier())
			{
				summary_data();
				statistics_data();
				if (need_sort)
				{
					sort_data();
				}
			}
			data_file.close();
		}
		catch (ofstream::failure)
		{
			output_error(FILE_ERROR);
		}
	}
	log_file.close();
}