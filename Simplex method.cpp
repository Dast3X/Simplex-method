#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <cmath>
#include <limits>
#include <numeric>
#include <valarray>

using namespace std;





class Simplex
{
public:
	Simplex(string DATA)
	{

		this->DATA = DATA;
		ofstream file;
		file.open("Solution\\Decimal\\Solution - " + DATA);
		file.close();
		file.open("Solution\\Fraction\\Solution - " + DATA);
		file.close();
		read(DATA);
		addSlacks();
		try
		{
			while (true)
			{
				write(DATA);
				writeFraction(DATA);
				doIteration();
			}
		}
		catch (const std::exception&) {}

		Result();
		iteration = 0;
		MultipleSolution();
		writeAnswer(DATA);
		writeAnswerFraction(DATA);
	}

private:

	void test_for_unboundedness(float value)
	{
		if (value == std::numeric_limits<float>::infinity())
			isUnbounded = true;
		if (value == -std::numeric_limits<float>::infinity())
			isUnbounded = true;
		if (value == std::numeric_limits<float>::quiet_NaN())
			isUnbounded = true;
		if (value == -std::numeric_limits<float>::quiet_NaN())
			isUnbounded = true;
		if (value == std::numeric_limits<float>::signaling_NaN())
			isUnbounded = true;
		if (value == -std::numeric_limits<float>::signaling_NaN())
			isUnbounded = true;
	}
	int indexOfMinNegative()
	{
		auto index = distance(z.begin(), min_element(z.begin(), z.end()));
		if (z[index] < 0)
			return index;
		else
			throw exception("Simplex Done");
	}
	int indexOfMinPositive(int index)
	{
		vector<float> positiveB;
		for (int i = 0; i < amountConstraints; i++)
		{
			if (g[i][index] > 0 && b[i] / g[i][index] >= 0)
				positiveB.push_back(b[i] / g[i][index]);
			else
				positiveB.push_back((float)99999999);
		}
		auto index2 = distance(positiveB.begin(), min_element(positiveB.begin(), positiveB.end()));
		return index2;
	}
	void read(string DATA)
	{
		fstream file;
		file.open("Problems\\" + DATA, ios::in, ios::binary);
		if (!file.is_open())
			throw exception("No files");
		string temp;
		getline(file, temp);
		float number;
		stringstream iss(temp);
		while (iss >> number)
		{
			z.push_back(number);
		}
		getline(file, temp);

		this->amountVariables = int(z.size());
		temp = " ";

		for (int i = 0; temp != ""; i++)
		{
			getline(file, temp);
			if (temp == "")
				break;
			stringstream iss(temp);
			g.push_back(vector<float>());
			while (iss >> number)
				g[i].push_back(number);
		}
		this->amountConstraints = int(g.size());

		getline(file, temp);
		stringstream oss(temp);
		while (oss >> number)
		{
			b.push_back(number);
		}
		if (amountConstraints < 2 || amountVariables < 2)
			throw exception("ERROR: incorrect problem statement: Amount of variables or constraints must be >= 2");
		if (g.size() != b.size())
			throw exception("ERROR: incorrect problem statement: b != g");




		file.close();
	}
	void addSlacks()
	{
		for (auto& i : z)
			i *= -1;
		for (int i = 0; i < amountConstraints + 1; i++)
			z.push_back(0);
		for (int i = 0; i < amountConstraints; i++)
		{
			for (int j = 0; j < amountConstraints; j++)
			{
				g[j].push_back(0);
			}
			g[i][i + amountVariables] = 1;
		}
		for (int i = 0; i < amountConstraints; i++)
		{
			rowLabels.push_back(("s" + to_string(i + 1)));
		}
	}
	void doIteration(int index = 0)
	{
		if (iteration > (amountVariables + 1) * 2)
		{
			isUnbounded = true;
			throw exception();
		}
		int cols = amountConstraints + amountVariables;
		int colsIndex;
		int rowIndex;
		if (isMultiple == false)
		{
			colsIndex = indexOfMinNegative();
			rowIndex = indexOfMinPositive(colsIndex);
		}
		else if (isMultiple == true)
		{
			colsIndex = index;
			rowIndex = indexOfMinPositive(colsIndex);
		}
		if (colsIndex < amountVariables) {
			rowLabels[rowIndex] = "x" + to_string(colsIndex + 1);
		}
		float crossingElement = g[rowIndex][colsIndex];
		b[rowIndex] /= crossingElement;
		for (int i = 0; i < cols; i++)
			g[rowIndex][i] /= crossingElement;
		for (int i = 0; i < amountConstraints; i++)
		{
			float temp = g[i][colsIndex];
			for (int j = 0; j < cols && i != rowIndex; j++)
				g[i][j] -= g[rowIndex][j] * temp;
			if (i != rowIndex)
				b[i] -= b[rowIndex] * temp;
		}
		float temp = z[colsIndex];
		for (int i = 0; i < cols; i++)
		{
			z[i] -= g[rowIndex][i] * temp;
		}
		z[cols] -= b[rowIndex] * temp;
		if (isObjVariablePositive() && isObjSlackNegative())
		{
			Result();
			isMultiple2 = true;
		}
		iteration++;
	}
	bool isObjVariablePositive()
	{
		for (size_t i = 0; i < amountVariables; i++)
			if (z[i] < 0)
				return false;
		return true;
	}
	bool isObjSlackNegative()
	{
		for (size_t i = amountVariables; i < amountConstraints + amountVariables; i++)
			if (z[i] < 0)
				return true;
		return false;
	}
	void MultipleSolution()
	{
		ofstream file;
		file.open("Solution\\Decimal\\Solution - " + DATA, ios::app);
		ofstream file2;
		file2.open("Solution\\Fraction\\Solution - " + DATA, ios::app);
		int cols = amountConstraints + amountVariables;
		float temp = 0;
		vector<vector <float>> gtemp = g;
		vector<float> ztemp = z;
		vector<float> btemp = b;
		for (int i = 0; i < cols; i++)
		{
			int jtemp;
			temp = 0;
			z = ztemp;
			int a;
			b = btemp;
			g = gtemp;
			if (z[i] == 0)
			{
				for (int j = 0; j < g.size(); j++)
				{
					temp += g[j][i];
					jtemp = j;
				}

				if (temp != 1)
				{
					isMultiple = true;
					file2 << "Another solution\n";
					file << "Another solution\n";
					file.close();
					file2.close();
					doIteration(i);
					write(DATA);
					writeFraction(DATA);
					Result();
				}

			}
		}

		return;
	}
	bool isNonBasis(int j, int itemp) {
		for (size_t i = 0; i < g.size(); i++)
			if (g[j][i] == 1)
				return false;
		return true;
	}
	void Result()
	{
		vector<float> btemp(amountConstraints + amountVariables);
		;

		for (int i = 0; i < g.size(); i++)
		{
			if (rowLabels[i][0] == 'x')
			{
				btemp[stringToInt(rowLabels[i]) - 1] = b[i];
			}
			else
			{
				btemp[stringToInt(rowLabels[i]) + amountVariables - 1] = b[i];
			}
		}
		variablesResult.push_back(btemp);
		objectiveResult.push_back(*(z.end() - 1));
	}
	int stringToInt(string str)
	{
		string temp;
		for (int i = 1; i < str.size(); i++)
			temp += str[i];
		return stoi(temp);
	}

	void write(string DATA)
	{
		ofstream file;
		file.open("Solution\\Decimal\\Solution - " + DATA, ios::app);

		int cols = amountConstraints + amountVariables + 1;
		for (int i = 0; i < amountVariables; i++)
			file << "\tx" << i + 1;
		for (int i = 0; i < amountConstraints; i++)
			file << "\ts" << i + 1;
		file << "\tb";
		for (int i = 0; i < amountConstraints; i++)
		{
			file << "\n" << rowLabels[i];
			for (int j = 0; j < cols - 1; j++)
			{
				file << "\t" << round(g[i][j] * 10) / 10;
			}
			file << '\t' << round(b[i] * 10) / 10;
		}
		file << "\nc";
		for (const auto& i : z)
		{
			file << "\t" << round(i * 10) / 10;
		}
		file << "\n\n";

		file.close();
	}
	void writeAnswer(string DATA)
	{
		ofstream file;
		file.open("Solution\\Decimal\\Solution - " + DATA, ios::app);
		file << "SOLUTION FOUND:";
		test_for_unboundedness(*(z.end() - 1));
		if (isUnbounded == true)
			file << " unbounded";
		else
		{
			if (isMultiple == true || isMultiple2 == true)
			{
				file << " multiple solution\n";
				file << "Objective: z = " << objectiveResult[1] << "\n  \t \t";
				for (int i = 0; i < amountVariables; i++)
					file << "x" << i + 1 << "\t";
				for (int i = 0; i < amountConstraints; i++)
					file << "s" << i + 1 << "\t";
				for (int i = 0; i < variablesResult.size(); i++)
				{
					file << "\nx*\t=\t";
					for (int j = 0; j < amountVariables + amountConstraints; j++)
						file << round(variablesResult[i][j] * 10) / 10 << '\t';
				}
			}
			else
			{
				file << " unique solution\n";
				file << "Objective: z = " << objectiveResult[0] << "\n  \t \t";
				for (int i = 0; i < amountVariables; i++)
					file << "x" << i + 1 << "\t";
				for (int i = 0; i < amountConstraints; i++)
					file << "s" << i + 1 << "\t";
				file << "\nx*\t=\t";
				for (int j = 0; j < amountVariables + amountConstraints; j++)
					file << round(variablesResult[0][j] * 10) / 10 << '\t';


			}
		}
		file.close();
	}

	void writeFraction(string DATA)
	{
		ofstream file;
		file.open("Solution\\Fraction\\Solution - " + DATA, ios::app);

		int cols = amountConstraints + amountVariables + 1;
		for (int i = 0; i < amountVariables; i++)
			file << "\tx" << i + 1;
		for (int i = 0; i < amountConstraints; i++)
			file << "\ts" << i + 1;
		file << "\tb";
		for (int i = 0; i < amountConstraints; i++)
		{
			file << "\n" << rowLabels[i];
			for (int j = 0; j < cols - 1; j++)
			{
				file << "\t" << to_fraction(g[i][j]);
			}
			file << '\t' << to_fraction(b[i]);
		}
		file << "\nc";
		for (const auto& i : z)
		{
			file << "\t" << to_fraction(i);
		}
		file << "\n\n";

		file.close();
	}
	void writeAnswerFraction(string DATA)
	{
		ofstream file;
		file.open("Solution\\Fraction\\Solution - " + DATA, ios::app);
		file << "SOLUTION FOUND:";
		test_for_unboundedness(*(z.end() - 1));
		if (isUnbounded == true)
			file << " unbounded";
		else
		{
			if (isMultiple == true || isMultiple2 == true)
			{
				file << " multiple solution\n";
				file << "Objective: z = " << to_fraction(objectiveResult[1]) << "\n  \t \t";
				for (int i = 0; i < amountVariables; i++)
					file << "x" << i + 1 << "\t";
				for (int i = 0; i < amountConstraints; i++)
					file << "s" << i + 1 << "\t";
				for (int i = 0; i < variablesResult.size(); i++)
				{
					file << "\nx*\t=\t";
					for (int j = 0; j < amountVariables + amountConstraints; j++)
						file << to_fraction(variablesResult[i][j]) << '\t';
				}
			}
			else
			{
				file << " unique solution\n";
				file << "Objective: z = " << to_fraction(objectiveResult[0]) << "\n  \t \t";
				for (int i = 0; i < amountVariables; i++)
					file << "x" << i + 1 << "\t";
				for (int i = 0; i < amountConstraints; i++)
					file << "s" << i + 1 << "\t";
				file << "\nx*\t=\t";
				for (int j = 0; j < amountVariables + amountConstraints; j++)
					file << to_fraction(variablesResult[0][j]) << '\t';


			}
		}
		file.close();
	}
	string to_fraction(float number, int cycles = 10, float precision = 5e-3) {
		if (number == 0)
			return "0";
		if (number == (int)number)
			return to_string((int)number);
		int sign = number > 0 ? 1 : -1;
		number = number * sign; //abs(number);
		float new_number, whole_part;
		float decimal_part = number - (int)number;
		int counter = 0;

		valarray<float> vec_1{ float((int)number), 1 }, vec_2{ 1,0 }, temporary;

		while (decimal_part > precision & counter < cycles) {
			new_number = 1 / decimal_part;
			whole_part = (int)new_number;

			temporary = vec_1;
			vec_1 = whole_part * vec_1 + vec_2;
			vec_2 = temporary;

			decimal_part = new_number - whole_part;
			counter += 1;
		}
		if (int(vec_1[1]) == 1)
			return to_string(int(sign * vec_1[0]));
		if (int(sign * vec_1[0]) == 0)
			return "0";

		return to_string(int(sign * vec_1[0])) + '/' + to_string(int(vec_1[1]));
	}

	vector<float> z;
	vector<vector<float>> g;
	vector<float> b;
	vector<vector<float>> variablesResult;
	vector<float> objectiveResult;
	vector<string> rowLabels;
	string DATA;
	int iteration = 0;
	int amountVariables;
	int amountConstraints;
	bool isUnbounded;
	bool isMultiple = false;
	bool isMultiple2 = false;
};




int main()
{
	string DATA;
	cout << "Please write the name of the file (e.g Input.txt):";
	getline(cin, DATA);
	try
	{
		Simplex smpl(DATA);
	}
	catch (const exception& a)
	{
		cout << a.what() << "\n\n";
		cin.ignore();
		return 0;
	}
	cout << "\n\t\tFile with solution saved in: \n\t\tSolution\\Decimal\\Solution - " + DATA
		<< "\n\t\tSolution\\Fraction\\Solution - " + DATA << "\n\n\n\n\n";
	cin.ignore();
}


