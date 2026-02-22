#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>
using namespace std;

struct Equation {
    string* varNames;
    float*  varCoeffs;
    int     varCount;
    int     arrayCapacity;
    float   rightHandSide;

    Equation() : varCount(0), arrayCapacity(10), rightHandSide(0) {
        varNames  = new string[arrayCapacity];
        varCoeffs = new float[arrayCapacity];
        for (int i = 0; i < arrayCapacity; i++) varCoeffs[i] = 0;
    }

    Equation(const Equation& other) : varCount(other.varCount), arrayCapacity(other.arrayCapacity), rightHandSide(other.rightHandSide) {
        varNames  = new string[arrayCapacity];
        varCoeffs = new float[arrayCapacity];
        for (int i = 0; i < arrayCapacity; i++) {
            varNames[i]  = other.varNames[i];
            varCoeffs[i] = other.varCoeffs[i];
        }
    }

    Equation& operator=(const Equation& other) {
        if (this == &other) return *this;
        delete[] varNames;
        delete[] varCoeffs;
        varCount  = other.varCount;
        arrayCapacity  = other.arrayCapacity;
        rightHandSide  = other.rightHandSide;
        varNames  = new string[arrayCapacity];
        varCoeffs = new float[arrayCapacity];
        for (int i = 0; i < arrayCapacity; i++) {
            varNames[i]  = other.varNames[i];
            varCoeffs[i] = other.varCoeffs[i];
        }
        return *this;
    }

    ~Equation() {
        delete[] varNames;
        delete[] varCoeffs;
    }

    void expandArrays() {
        arrayCapacity *= 2;
        string* newNames  = new string[arrayCapacity];
        float*  newCoeffs = new float[arrayCapacity];
        for (int i = 0; i < varCount; i++) {
            newNames[i]  = varNames[i];
            newCoeffs[i] = varCoeffs[i];
        }
        delete[] varNames;
        delete[] varCoeffs;
        varNames  = newNames;
        varCoeffs = newCoeffs;
    }

    int findVar(const string& name) const {
        for (int i = 0; i < varCount; i++)
            if (varNames[i] == name) return i;
        return -1;
    }

    void addToCoeff(const string& name, float value) {
        int index = findVar(name);
        if (index == -1) {
            if (varCount == arrayCapacity) expandArrays();
            varNames[varCount]  = name;
            varCoeffs[varCount] = value;
            varCount++;
        } else {
            varCoeffs[index] += value;
        }
    }

    float getCoeff(const string& name) const {
        int index = findVar(name);
        return (index == -1) ? 0 : varCoeffs[index];
    }

    void sortAlphabetically() {
        for (int i = 0; i < varCount - 1; i++)
            for (int j = 0; j < varCount - i - 1; j++)
                if (varNames[j] > varNames[j+1]) {
                    string tempName       = varNames[j];
                    varNames[j]      = varNames[j+1];
                    varNames[j+1]    = tempName;
                    float tempCoeff       = varCoeffs[j];
                    varCoeffs[j]     = varCoeffs[j+1];
                    varCoeffs[j+1]   = tempCoeff;
                }
    }
};

string equationToString(Equation& eq) {
    eq.sortAlphabetically();
    ostringstream output;
    bool isFirstTerm = true;
    for (int i = 0; i < eq.varCount; i++) {
        float coeff = eq.varCoeffs[i];
        if (coeff == 0) continue;
        if (!isFirstTerm && coeff > 0) output << "+";
        if (coeff == (int)coeff) output << (int)coeff;
        else                     output << coeff;
        output << eq.varNames[i];
        isFirstTerm = false;
    }
    if (isFirstTerm) output << "0";
    output << "=";
    if (eq.rightHandSide == (int)eq.rightHandSide) output << (int)eq.rightHandSide;
    else                                            output << eq.rightHandSide;
    return output.str();
}

void parseLeftSide(const string& leftSide, Equation& eq) {
    string input = leftSide;
    if (input.empty()) return;
    if (input[0] != '-') input = "+" + input;

    string* terms    = new string[200];
    int     termCount = 0;
    string  currentTerm = "";

    for (int i = 0; i < (int)input.size(); i++) {
        if ((input[i] == '+' || input[i] == '-') && i != 0) {
            if (!currentTerm.empty()) terms[termCount++] = currentTerm;
            currentTerm = input[i];
        } else currentTerm += input[i];
    }
    if (!currentTerm.empty()) terms[termCount++] = currentTerm;

    for (int t = 0; t < termCount; t++) {
        string& term = terms[t];
        int letterPos = -1;
        for (int i = 0; i < (int)term.size(); i++)
            if (term[i] >= 'a' && term[i] <= 'z') { letterPos = i; break; }

        if (letterPos == -1) {
            eq.rightHandSide -= atof(term.c_str());
        } else {
            string coeffPart = term.substr(0, letterPos);
            string varName   = term.substr(letterPos);
            float  coeff;
            if (coeffPart.empty() || coeffPart == "+") coeff = 1;
            else if (coeffPart == "-")                  coeff = -1;
            else                                        coeff = atof(coeffPart.c_str());
            eq.addToCoeff(varName, coeff);
        }
    }
    delete[] terms;
}

Equation* allEquations;
int       equationCount;

int collectAllVariables(string*& outVars) {
    int     capacity  = 10;
    string* uniqueVars = new string[capacity];
    int     uniqueCount = 0;

    for (int i = 0; i < equationCount; i++)
        for (int j = 0; j < allEquations[i].varCount; j++) {
            bool alreadyExists = false;
            for (int k = 0; k < uniqueCount; k++)
                if (uniqueVars[k] == allEquations[i].varNames[j]) { alreadyExists = true; break; }
            if (!alreadyExists) {
                if (uniqueCount == capacity) {
                    capacity *= 2;
                    string* expanded = new string[capacity];
                    for (int x = 0; x < uniqueCount; x++) expanded[x] = uniqueVars[x];
                    delete[] uniqueVars;
                    uniqueVars = expanded;
                }
                uniqueVars[uniqueCount++] = allEquations[i].varNames[j];
            }
        }

    for (int i = 0; i < uniqueCount - 1; i++)
        for (int j = 0; j < uniqueCount - i - 1; j++)
            if (uniqueVars[j] > uniqueVars[j+1]) { string t = uniqueVars[j]; uniqueVars[j] = uniqueVars[j+1]; uniqueVars[j+1] = t; }

    outVars = uniqueVars;
    return uniqueCount;
}

float calcDeterminant(float** matrix, int size) {
    if (size == 1) return matrix[0][0];
    if (size == 2) return matrix[0][0]*matrix[1][1] - matrix[0][1]*matrix[1][0];
    float result = 0;
    float** subMatrix = new float*[size-1];
    for (int i = 0; i < size-1; i++) subMatrix[i] = new float[size-1];
    for (int col = 0; col < size; col++) {
        for (int row = 1; row < size; row++) {
            int subCol = 0;
            for (int c = 0; c < size; c++)
                if (c != col) subMatrix[row-1][subCol++] = matrix[row][c];
        }
        result += matrix[0][col] * calcDeterminant(subMatrix, size-1) * (col%2==0 ? 1 : -1);
    }
    for (int i = 0; i < size-1; i++) delete[] subMatrix[i];
    delete[] subMatrix;
    return result;
}

float** buildCoeffMatrix(string* varNames, int size, string replaceVar = "") {
    float** matrix = new float*[equationCount];
    for (int row = 0; row < equationCount; row++) {
        matrix[row] = new float[size];
        for (int col = 0; col < size; col++)
            matrix[row][col] = (!replaceVar.empty() && varNames[col] == replaceVar)
                               ? allEquations[row].rightHandSide
                               : allEquations[row].getCoeff(varNames[col]);
    }
    return matrix;
}

void freeMatrix(float** matrix) {
    for (int i = 0; i < equationCount; i++) delete[] matrix[i];
    delete[] matrix;
}

void printNumber(float value) {
    if (value == (int)value) cout << (int)value;
    else                     cout << value;
}

int main() {
    cin >> equationCount;
    allEquations = new Equation[equationCount];

    for (int i = 0; i < equationCount; i++) {
        string line;
        cin >> ws;
        getline(cin, line);
        size_t equalSign             = line.find('=');
        allEquations[i].rightHandSide = atof(line.substr(equalSign+1).c_str());
        parseLeftSide(line.substr(0, equalSign), allEquations[i]);
        allEquations[i].sortAlphabetically();
    }

    string command;
    while (cin >> command) {
        if (command == "quit") break;

        else if (command == "num_vars") {
            string* varList = nullptr;
            int count = collectAllVariables(varList);
            cout << count << endl;
            delete[] varList;
        }

        else if (command == "equation") {
            int index; cin >> index;
            cout << equationToString(allEquations[index-1]) << endl;
        }

        else if (command == "column") {
            string varName; cin >> varName;
            for (int i = 0; i < equationCount; i++) {
                printNumber(allEquations[i].getCoeff(varName));
                cout << endl;
            }
        }

        else if (command == "add") {
            int i, j; cin >> i >> j;
            Equation result = allEquations[i-1];
            result.rightHandSide += allEquations[j-1].rightHandSide;
            for (int k = 0; k < allEquations[j-1].varCount; k++)
                result.addToCoeff(allEquations[j-1].varNames[k], allEquations[j-1].varCoeffs[k]);
            cout << equationToString(result) << endl;
        }

        else if (command == "subtract") {
            int i, j; cin >> i >> j;
            Equation result = allEquations[i-1];
            result.rightHandSide -= allEquations[j-1].rightHandSide;
            for (int k = 0; k < allEquations[j-1].varCount; k++)
                result.addToCoeff(allEquations[j-1].varNames[k], -allEquations[j-1].varCoeffs[k]);
            cout << equationToString(result) << endl;
        }

        else if (command == "substitute") {
            string targetVar; int eqIndex, subIndex;
            cin >> targetVar >> eqIndex >> subIndex;
            Equation& sourceEq = allEquations[eqIndex-1];
            Equation& subEq    = allEquations[subIndex-1];
            float coeffInSource = sourceEq.getCoeff(targetVar);
            float coeffInSub    = subEq.getCoeff(targetVar);
            if (coeffInSource == 0 || coeffInSub == 0) { cout << equationToString(sourceEq) << endl; continue; }
            float scaleFactor = coeffInSource / coeffInSub;
            Equation result   = sourceEq;
            result.rightHandSide -= scaleFactor * subEq.rightHandSide;
            for (int k = 0; k < subEq.varCount; k++)
                result.addToCoeff(subEq.varNames[k], -scaleFactor * subEq.varCoeffs[k]);
            int idx = result.findVar(targetVar);
            if (idx != -1) result.varCoeffs[idx] = 0;
            cout << equationToString(result) << endl;
        }

        else if (command == "D") {
            string rest, replaceVar = "";
            getline(cin, rest);
            stringstream ss(rest);
            ss >> replaceVar;

            string* varList = nullptr;
            int size = collectAllVariables(varList);
            float** matrix = buildCoeffMatrix(varList, size, replaceVar);
            for (int row = 0; row < equationCount; row++) {
                for (int col = 0; col < size; col++) {
                    printNumber(matrix[row][col]);
                    if (col < size-1) cout << "\t";
                }
                cout << endl;
            }
            freeMatrix(matrix);
            delete[] varList;
        }

        else if (command == "D_value") {
            string* varList = nullptr;
            int size = collectAllVariables(varList);
            float** matrix = buildCoeffMatrix(varList, size);
            printNumber(calcDeterminant(matrix, size));
            cout << endl;
            freeMatrix(matrix);
            delete[] varList;
        }

        else if (command == "solve") {
            string* varList = nullptr;
            int size = collectAllVariables(varList);
            float** matrix = buildCoeffMatrix(varList, size);
            float mainDet  = calcDeterminant(matrix, size);
            if (mainDet == 0) {
                cout << "No Solution" << endl;
            } else {
                for (int k = 0; k < size; k++) {
                    float** cramersMatrix = buildCoeffMatrix(varList, size, varList[k]);
                    cout << varList[k] << "=";
                    printNumber(calcDeterminant(cramersMatrix, size) / mainDet);
                    cout << endl;
                    freeMatrix(cramersMatrix);
                }
            }
            freeMatrix(matrix);
            delete[] varList;
        }
    }

    delete[] allEquations;
    return 0;
}