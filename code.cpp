#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <ctime>
#include <iomanip>
#include "date.h" // Include the date library header

using namespace std;
using namespace date; // Use the date namespace

// Define a struct to hold employee information
struct Employee
{
    string name;
    sys_seconds startTime;
    sys_seconds endTime;
};

int main()
{
    ifstream inputFile("input.csv");
    ofstream outputFile("output.txt");

    if (!inputFile)
    {
        cerr << "Error: Unable to open input file." << endl;
        return 1;
    }

    if (!outputFile)
    {
        cerr << "Error: Unable to open output file." << endl;
        return 1;
    }

    // Create containers to store employee data and analysis results
    vector<Employee> employees;
    map<string, sys_seconds> employeeLastEndTime;
    map<string, int> consecutiveDaysWorked;

    string line;
    getline(inputFile, line);

    while (getline(inputFile, line))
    {
        stringstream ss(line);
        string positionID, positionStatus, timeIn, timeOut, timecardHours, startDate, endDate, employeeName, fileNumber;

        getline(ss, positionID, ',');
        getline(ss, positionStatus, ',');
        getline(ss, timeIn, ',');
        getline(ss, timeOut, ',');
        getline(ss, timecardHours, ',');
        getline(ss, startDate, ',');
        getline(ss, endDate, ',');
        getline(ss, employeeName, ',');
        getline(ss, fileNumber, ',');

        // Parse timeIn and timeOut into sys_seconds
        struct tm tm
        {
        };
        std::istringstream timeInStream(timeIn);
        std::istringstream timeOutStream(timeOut);

        timeInStream >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
        sys_seconds startTime = sys_days{year{tm.tm_year + 1900} / (tm.tm_mon + 1) / tm.tm_mday} + std::chrono::hours{tm.tm_hour} + std::chrono::minutes{tm.tm_min} + std::chrono::seconds{tm.tm_sec};

        timeOutStream >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
        sys_seconds endTime = sys_days{year{tm.tm_year + 1900} / (tm.tm_mon + 1) / tm.tm_mday} + std::chrono::hours{tm.tm_hour} + std::chrono::minutes{tm.tm_min} + std::chrono::seconds{tm.tm_sec};

        // Create an Employee object and add it to the vector
        Employee employee = {employeeName, startTime, endTime};
        employees.push_back(employee);
    }

    // Analyze employee data
    for (const Employee &employee : employees)
    {
        string name = employee.name;
        sys_seconds startTime = employee.startTime;
        sys_seconds endTime = employee.endTime;

        // Check consecutive days worked
        auto lastEndTime = employeeLastEndTime[name];
        if (lastEndTime.time_since_epoch().count() != 0)
        {
            date::days daysBetween = date::floor<date::days>((startTime - lastEndTime));
            consecutiveDaysWorked[name]++;
            if (daysBetween > date::days{1})
            {
                consecutiveDaysWorked[name] = 1;
            }
        }

        // Check less than 10 hours between shifts
        if (lastEndTime.time_since_epoch().count() != 0 &&
            (startTime - lastEndTime) < std::chrono::hours{10} &&
            (startTime - lastEndTime) > std::chrono::hours{1})
        {
            outputFile << name << " has less than 10 hours between shifts." << endl;
        }

        // Check more than 14 hours in a single shift
        if ((endTime - startTime) > std::chrono::hours{14})
        {
            outputFile << name << " has worked for more than 14 hours in a single shift." << endl;
        }

        employeeLastEndTime[name] = endTime;
    }

    // Print employees who have worked for 7 consecutive days
    for (const auto &entry : consecutiveDaysWorked)
    {
        if (entry.second >= 7)
        {
            outputFile << entry.first << " has worked for 7 consecutive days." << endl;
        }
    }

    inputFile.close();
    outputFile.close();

    return 0;
}
