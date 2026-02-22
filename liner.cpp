/*
 * Linear Equations Solver
 * ========================
 * This program reads a system of linear equations and performs
 * various operations on them like adding, subtracting, and solving.
 *
 * We only use <iostream> for input/output (cin, cout)
 * and <string> for text handling.
 * Everything else (like converting numbers, sorting, etc.)
 * we build ourselves from scratch.
 */

#include <iostream>   // needed for cin (input) and cout (output)
#include <string>     // needed for the string data type
using namespace std;  // so we don't have to write std::cout every time


/*
 * myAtof - converts a string like "3.5" or "-2" into a float number
 *
 * Why we need it:
 *   The standard function atof() requires #include<cstdlib>
 *   which we are not using, so we build our own version.
 *
 * How it works:
 *   We read each character one by one.
 *   If we see '-' at the start, the number is negative.
 *   If we see digits before '.', we build the integer part.
 *   If we see digits after '.', we build the decimal part.
 */
float myAtof(const string &s) {
    float result   = 0;      // will hold the integer part of the number
    float dec      = 0;      // will hold the decimal part of the number
    bool negative  = false;  // is the number negative?
    bool afterDot  = false;  // have we passed the decimal point '.' yet?
    float decPlace = 0.1f;   // each decimal digit is multiplied by 0.1, 0.01, 0.001 ...
    int i = 0;               // index to walk through the string

    // check if the number starts with a sign
    if (!s.empty() && s[0] == '-') { negative = true; i = 1; }  // skip the '-'
    else if (!s.empty() && s[0] == '+') { i = 1; }              // skip the '+'

    // go through each character of the string
    for (; i < (int)s.size(); i++) {
        if (s[i] == '.') {
            afterDot = true;  // from now on, digits go to the decimal part
            continue;
        }
        if (!afterDot) {
            // integer part: shift existing digits left and add new digit
            // example: result=3, digit=5 => result = 3*10 + 5 = 35
            result = result * 10 + (s[i] - '0');
            // note: s[i]-'0' converts character '5' to integer 5
        } else {
            // decimal part: multiply digit by its place value (0.1, 0.01, ...)
            dec += (s[i] - '0') * decPlace;
            decPlace *= 0.1f;  // next digit is 10x smaller
        }
    }

    result += dec;                    // combine integer and decimal parts
    return negative ? -result : result; // apply sign and return
}


/*
 * floatToStr - converts a float number like 3.5 into a string "3.5"
 *
 * Why we need it:
 *   Normally we use ostringstream from <sstream> to do this,
 *   but we are not allowed to use that library.
 *   So we build the string digit by digit ourselves.
 *
 * How it works:
 *   First handle negative numbers by adding '-' and flipping the sign.
 *   Then extract the integer part and convert each digit to a character.
 *   Then extract up to 4 decimal digits if needed.
 */
string floatToStr(float v) {
    // handle negative numbers: add '-' then convert the positive version
    if (v < 0) return "-" + floatToStr(-v);

    int intPart = (int)v;       // get the integer part (e.g. 3.75 -> 3)
    float frac  = v - intPart;  // get the fractional part (e.g. 3.75 -> 0.75)

    string res = "";   // this will be our result string
    int tmp = intPart; // temporary copy to extract digits
    string rev = "";   // digits come out in reverse order, so we store them here

    if (tmp == 0) rev = "0"; // special case: number is 0

    // extract digits one by one from the right
    // example: 123 -> first digit extracted is '3', then '2', then '1'
    while (tmp > 0) {
        rev += (char)('0' + tmp % 10); // tmp%10 gives last digit, +'0' makes it a char
        tmp /= 10;                      // remove last digit
    }

    // reverse the string to get correct order
    // example: "321" becomes "123"
    for (int i = rev.size()-1; i >= 0; i--) res += rev[i];

    // now handle decimal part if it exists
    if (frac > 0.0001f) {  // only add decimal if it is meaningful (not just rounding error)
        res += ".";
        for (int i = 0; i < 4; i++) {  // print up to 4 decimal digits
            frac *= 10;                 // shift one decimal place left
            res += (char)('0' + (int)frac); // add the digit
            frac -= (int)frac;          // remove the digit we just added
            if (frac < 0.0001f) break;  // stop if no more meaningful digits
        }
    }

    return res;
}


/*
 * We define maximum sizes for arrays.
 * Since we are not allowed to use vector (which can grow automatically),
 * we use fixed-size arrays and these constants tell us the maximum size.
 */
const int MAXVARS = 100;  // maximum number of different variables (x1, x2, ...)
const int MAXEQS  = 100;  // maximum number of equations


/*
 * Equation struct - represents one linear equation
 *
 * Example equation: 2x1 + 3x2 - x3 = 10
 *
 * We store it as:
 *   varNames = ["x1", "x2", "x3"]
 *   coeffs   = [2, 3, -1]
 *   numVars  = 3
 *   constant = 10
 *
 * Why not use map<string, float>?
 *   map requires #include<map> which we are not using.
 *   Instead we use two parallel arrays: one for names, one for values.
 *   "Parallel arrays" means varNames[i] and coeffs[i] always belong together.
 */
struct Equation {
    string varNames[MAXVARS];  // names of variables (e.g. "x1", "x2")
    float  coeffs[MAXVARS];    // coefficient for each variable
    int    numVars;            // how many variables are currently stored
    float  constant;           // the number on the right side of '='

    /*
     * Constructor - runs automatically when we create an Equation object.
     * It sets everything to zero so we start with a clean slate.
     */
    Equation() : numVars(0), constant(0) {
        for (int i = 0; i < MAXVARS; i++) coeffs[i] = 0;
    }

    /*
     * findVar - searches for a variable name and returns its index.
     * Returns -1 if the variable is not found.
     *
     * Example: if varNames = ["x1","x2","x3"] and we search for "x2",
     *          it returns 1 (index of x2 in the array)
     */
    int findVar(const string &v) const {
        for (int i = 0; i < numVars; i++)
            if (varNames[i] == v) return i;  // found it, return the index
        return -1;  // not found
    }

    /*
     * addCoeff - adds a coefficient to a variable.
     * If the variable already exists, it adds to the existing coefficient.
     * If the variable is new, it creates a new entry.
     *
     * Why "adds" and not "sets"?
     *   Because the same variable might appear multiple times in input
     *   like: 4x1 + 3x2 - 2x1 = 10
     *   Here x1 appears twice, so we add: 4 + (-2) = 2x1
     */
    void addCoeff(const string &v, float c) {
        int idx = findVar(v);
        if (idx == -1) {
            // variable not found: add it as a new entry at the end
            varNames[numVars] = v;
            coeffs[numVars]   = c;
            numVars++;
        } else {
            // variable already exists: add to its current coefficient
            coeffs[idx] += c;
        }
    }

    /*
     * getCoeff - returns the coefficient of a variable.
     * Returns 0 if the variable is not in this equation
     * (which means it has coefficient 0, i.e. it doesn't appear).
     */
    float getCoeff(const string &v) const {
        int idx = findVar(v);
        return (idx == -1) ? 0 : coeffs[idx];
    }

    /*
     * setCoeff - sets the coefficient of a variable to an exact value.
     * Unlike addCoeff, this replaces the old value instead of adding to it.
     * Used when we need to force a variable's coefficient to a specific number.
     */
    void setCoeff(const string &v, float c) {
        int idx = findVar(v);
        if (idx == -1) {
            // new variable: create entry
            varNames[numVars] = v;
            coeffs[numVars]   = c;
            numVars++;
        } else {
            // existing variable: overwrite its coefficient
            coeffs[idx] = c;
        }
    }

    /*
     * sortVars - sorts variables alphabetically (a to z).
     *
     * Why do we need this?
     *   The project requires equations to be printed with variables
     *   in alphabetical order. Example: x1 before x2 before x3.
     *
     * How it works:
     *   We use "Bubble Sort" - a simple sorting algorithm.
     *   We compare each pair of neighbors and swap them if they are
     *   in the wrong order. We repeat until everything is sorted.
     *   When we swap variable names, we also swap their coefficients
     *   to keep them matched (parallel arrays must stay in sync).
     */
    void sortVars() {
        for (int i = 0; i < numVars-1; i++) {
            for (int j = 0; j < numVars-i-1; j++) {
                if (varNames[j] > varNames[j+1]) {
                    // swap the names
                    string tmpN    = varNames[j];
                    varNames[j]    = varNames[j+1];
                    varNames[j+1]  = tmpN;
                    // swap the coefficients (must stay matched with names)
                    float tmpC   = coeffs[j];
                    coeffs[j]    = coeffs[j+1];
                    coeffs[j+1]  = tmpC;
                }
            }
        }
    }
};


/*
 * printEquation - converts an Equation object into a readable string.
 *
 * Example output: "2x1+3x2-1x3=10"
 *
 * Rules we follow (from project requirements):
 *   1. Variables must be in alphabetical order (we call sortVars first)
 *   2. Skip variables with coefficient 0 (they don't appear in the equation)
 *   3. Add '+' between terms only if the next coefficient is positive
 *      (negative coefficients already have '-' built in)
 *   4. The constant goes on the right side of '='
 */
string printEquation(Equation &eq) {
    eq.sortVars();  // always sort before printing to ensure correct order

    string out  = "";    // result string we build character by character
    bool first  = true;  // tracks if this is the first term (no '+' before first term)

    for (int i = 0; i < eq.numVars; i++) {
        float c = eq.coeffs[i];
        if (c == 0) continue;  // skip variables with 0 coefficient

        // add '+' before positive terms (but not before the very first term)
        if (!first && c > 0) out += "+";

        // add the coefficient and variable name
        if (c < 0) out += "-" + floatToStr(-c);  // negative: add '-' then the absolute value
        else       out += floatToStr(c);           // positive: just add the number

        out += eq.varNames[i];  // add the variable name (e.g. "x1")
        first = false;          // next term is no longer the first
    }

    if (first) out += "0";  // edge case: all coefficients were 0, print "0"

    // add the '=' sign and the constant on the right side
    out += "=";
    if (eq.constant < 0) out += "-" + floatToStr(-eq.constant);
    else                  out += floatToStr(eq.constant);

    return out;
}


/*
 * parseLeft - reads the left side of an equation and fills the Equation struct.
 *
 * Example input string: "2x1+3x2-4x3"
 *
 * Step 1: Add '+' at the beginning so every term starts with a sign.
 *         "2x1+3x2-4x3" becomes "+2x1+3x2-4x3"
 *         This makes it easier to split into terms.
 *
 * Step 2: Split into individual terms by finding '+' and '-' signs.
 *         "+2x1", "+3x2", "-4x3"
 *
 * Step 3: For each term, separate the coefficient from the variable name.
 *         "+2x1" -> coefficient=2, variable="x1"
 *         "-4x3" -> coefficient=-4, variable="x3"
 *
 * Step 4: If a term has no letters (e.g. "+5"), it is a constant number.
 *         We move it to the right side of '=' by subtracting from constant.
 *         Example: 2x1+5=10 is the same as 2x1=5
 */
void parseLeft(const string &left, Equation &eq) {
    string s = left;
    if (s.empty()) return;

    // Step 1: ensure first term also has a sign, so we can split uniformly
    if (s[0] != '-') s = "+" + s;

    // Step 2: split the string into terms
    string terms[MAXVARS * 2];  // array to hold each term as a separate string
    int numTerms = 0;
    string cur = "";  // current term being built

    for (int i = 0; i < (int)s.size(); i++) {
        if ((s[i] == '+' || s[i] == '-') && i != 0) {
            // we found a new sign, so the current term is complete
            if (!cur.empty()) terms[numTerms++] = cur;
            cur = s[i];  // start a new term with this sign
        } else {
            cur += s[i];  // keep building the current term
        }
    }
    if (!cur.empty()) terms[numTerms++] = cur;  // don't forget the last term

    // Step 3 & 4: process each term
    for (int t = 0; t < numTerms; t++) {
        string &term = terms[t];

        // find the first letter in the term (start of variable name)
        int xPos = -1;
        for (int i = 0; i < (int)term.size(); i++) {
            if (term[i] >= 'a' && term[i] <= 'z') { xPos = i; break; }
        }

        if (xPos == -1) {
            // no letters found -> this term is a constant number
            // move it to the right side by subtracting from eq.constant
            // Example: "2x1+5=10" means 2x1 = 10-5 = 5
            eq.constant -= myAtof(term);
        } else {
            // letters found -> this is a variable term
            string coeffStr = term.substr(0, xPos); // part before the variable (coefficient)
            string var      = term.substr(xPos);    // part from first letter onward (variable name)

            // determine the coefficient value
            float coeff;
            if (coeffStr.empty() || coeffStr == "+") coeff = 1;   // "x1" or "+x1" means coefficient is 1
            else if (coeffStr == "-")                 coeff = -1;  // "-x1" means coefficient is -1
            else                                      coeff = myAtof(coeffStr); // convert string to float

            eq.addCoeff(var, coeff);  // store the variable and its coefficient
        }
    }
}


/*
 * We store all equations in a dynamically allocated array on the heap.
 *
 * Why the heap (using 'new')?
 *   We don't know how many equations the user will enter until runtime.
 *   Stack arrays need a fixed size at compile time.
 *   Heap memory (via 'new') lets us allocate exactly as much as we need.
 *
 * We also store 'n' (number of equations) globally so all functions can use it.
 */
Equation *equations;  // pointer to the array of equations (allocated later with 'new')
int n;                // number of equations (read from input)


/*
 * getAllVars - collects all unique variable names from all equations,
 *             sorted alphabetically, and stores them in the 'vars' array.
 * Returns the total number of unique variables found.
 *
 * Why do we need this?
 *   Different equations might have different variables.
 *   For operations like D_value and solve, we need a complete list
 *   of all variables across all equations.
 *
 * How it works:
 *   Loop through every equation and every variable in that equation.
 *   Before adding a variable to our list, check it's not already there.
 *   After collecting all variables, sort them alphabetically.
 */
int getAllVars(string *vars) {
    string tmp[MAXVARS];  // temporary array to collect unique variable names
    int cnt = 0;          // count of unique variables found so far

    for (int i = 0; i < n; i++) {                          // for each equation
        for (int j = 0; j < equations[i].numVars; j++) {  // for each variable in that equation
            bool found = false;

            // check if this variable is already in our list
            for (int k = 0; k < cnt; k++) {
                if (tmp[k] == equations[i].varNames[j]) { found = true; break; }
            }

            if (!found) tmp[cnt++] = equations[i].varNames[j];  // add if new
        }
    }

    // sort the collected variables alphabetically (bubble sort)
    for (int i = 0; i < cnt-1; i++)
        for (int j = 0; j < cnt-i-1; j++)
            if (tmp[j] > tmp[j+1]) { string t = tmp[j]; tmp[j] = tmp[j+1]; tmp[j+1] = t; }

    // copy sorted result into the output array
    for (int i = 0; i < cnt; i++) vars[i] = tmp[i];
    return cnt;
}


/*
 * determinant - calculates the determinant of a square matrix.
 *
 * Why do we need it?
 *   Cramer's Rule (used in 'solve') requires computing determinants.
 *   For a 2x2 matrix [[a,b],[c,d]], determinant = a*d - b*c
 *   For larger matrices, we use "cofactor expansion" (recursive approach).
 *
 * Parameters:
 *   mat - a 2D array of floats representing the matrix (allocated on heap)
 *   sz  - the size of the matrix (sz x sz)
 *
 * Why is mat a float** (pointer to pointer)?
 *   We cannot use vector, so we allocate 2D arrays on the heap manually.
 *   float** means: a pointer to an array of pointers, where each pointer
 *   points to one row of the matrix.
 *
 * How cofactor expansion works:
 *   Pick the first row. For each element in that row:
 *     - Create a smaller matrix by removing the current row and column.
 *     - Multiply the element by the determinant of the smaller matrix.
 *     - Alternate adding and subtracting (+ - + - ...).
 *   This is recursive: the smaller matrix's determinant is found the same way.
 *   Base cases: 1x1 matrix (just return the one element),
 *               2x2 matrix (use the simple formula).
 */
float determinant(float **mat, int sz) {
    // base case 1: 1x1 matrix
    if (sz == 1) return mat[0][0];

    // base case 2: 2x2 matrix - use direct formula
    if (sz == 2) return mat[0][0]*mat[1][1] - mat[0][1]*mat[1][0];

    // recursive case: cofactor expansion along the first row
    float det = 0;

    // allocate a smaller matrix on the heap for the sub-matrix
    // sub is (sz-1) x (sz-1) because we remove one row and one column
    float **sub = new float*[sz-1];
    for (int i = 0; i < sz-1; i++) sub[i] = new float[sz-1];

    // expand along each element in the first row
    for (int p = 0; p < sz; p++) {
        // build the sub-matrix by excluding row 0 and column p
        for (int i = 1; i < sz; i++) {  // start from row 1 (skip row 0)
            int col = 0;
            for (int j = 0; j < sz; j++) {
                if (j == p) continue;        // skip column p
                sub[i-1][col++] = mat[i][j]; // fill sub-matrix
            }
        }

        // add this term to the determinant
        // sign alternates: + for even p, - for odd p
        det += mat[0][p] * determinant(sub, sz-1) * (p % 2 == 0 ? 1 : -1);
    }

    // free the sub-matrix memory to avoid memory leaks
    for (int i = 0; i < sz-1; i++) delete[] sub[i];
    delete[] sub;

    return det;
}


int main() {
    // --- Read number of equations ---
    cin >> n;

    // Allocate array of n equations on the heap
    // 'new' reserves memory at runtime based on the value of n
    equations = new Equation[n];

    // --- Read each equation ---
    for (int i = 0; i < n; i++) {
        string line;
        cin >> ws;        // skip any whitespace (including newline) before reading
        getline(cin, line); // read the whole line including spaces

        // find '=' to split the equation into left and right sides
        size_t pos  = line.find('=');
        string left  = line.substr(0, pos);    // everything before '='
        string right = line.substr(pos + 1);   // everything after '='

        equations[i].constant = myAtof(right); // store the right-side constant
        parseLeft(left, equations[i]);          // parse left side into coefficients
        equations[i].sortVars();               // sort variables alphabetically
    }

    // --- Read and process commands ---
    string command;
    while (cin >> command) {

        // "quit" - end the program
        if (command == "quit") break;

        // -------------------------------------------------------
        // "num_vars" - print the total number of unique variables
        // -------------------------------------------------------
        else if (command == "num_vars") {
            string vars[MAXVARS];
            cout << getAllVars(vars) << endl;
        }

        // -------------------------------------------------------
        // "equation i" - print equation number i
        // -------------------------------------------------------
        else if (command == "equation") {
            int idx;
            cin >> idx;
            // idx is 1-based (user says "equation 1"), array is 0-based, so we use idx-1
            cout << printEquation(equations[idx - 1]) << endl;
        }

        // -------------------------------------------------------
        // "column x2" - print the coefficient of variable x2 in each equation
        // -------------------------------------------------------
        else if (command == "column") {
            string var;
            cin >> var;  // read variable name (e.g. "x2")
            for (int i = 0; i < n; i++) {
                float v = equations[i].getCoeff(var);
                // print without decimal point if it's a whole number
                if (v == (int)v) cout << (int)v << endl;
                else             cout << v << endl;
            }
        }

        // -------------------------------------------------------
        // "add i j" - add equation i and equation j, print result
        // -------------------------------------------------------
        else if (command == "add") {
            int i, j;
            cin >> i >> j;

            // start with a copy of equation i
            Equation res = equations[i - 1];
            res.constant += equations[j - 1].constant;  // add the constants

            // add each coefficient from equation j into the result
            for (int k = 0; k < equations[j - 1].numVars; k++)
                res.addCoeff(equations[j - 1].varNames[k], equations[j - 1].coeffs[k]);

            cout << printEquation(res) << endl;
        }

        // -------------------------------------------------------
        // "subtract i j" - subtract equation j from equation i, print result
        // -------------------------------------------------------
        else if (command == "subtract") {
            int i, j;
            cin >> i >> j;

            // start with a copy of equation i
            Equation res = equations[i - 1];
            res.constant -= equations[j - 1].constant;  // subtract the constants

            // subtract each coefficient of equation j from the result
            // subtracting is the same as adding the negative value
            for (int k = 0; k < equations[j - 1].numVars; k++)
                res.addCoeff(equations[j - 1].varNames[k], -equations[j - 1].coeffs[k]);

            cout << printEquation(res) << endl;
        }

        // -------------------------------------------------------
        // "substitute var i j" - remove variable 'var' from equation i
        //                        by substituting equation j in its place
        // -------------------------------------------------------
        else if (command == "substitute") {
            string var;
            int eqIdx, subIdx;
            cin >> var >> eqIdx >> subIdx;

            Equation &eq1 = equations[eqIdx - 1];  // equation we want to modify
            Equation &eq3 = equations[subIdx - 1];  // equation used for substitution

            float c1 = eq1.getCoeff(var);  // coefficient of 'var' in equation 1
            float c3 = eq3.getCoeff(var);  // coefficient of 'var' in equation 3

            // if 'var' doesn't appear in eq1, nothing to substitute
            if (c1 == 0) { cout << printEquation(eq1) << endl; continue; }
            // if 'var' doesn't appear in eq3, we can't eliminate it
            if (c3 == 0) { cout << printEquation(eq1) << endl; continue; }

            // calculate the factor we need to multiply eq3 by
            // so that when subtracted, the variable 'var' cancels out
            // Example: eq1 has 3x2, eq3 has 2x2 -> factor = 3/2
            //          subtract (3/2 * eq3) from eq1 -> x2 term becomes 0
            float factor = c1 / c3;

            // start with a copy of eq1
            Equation res = eq1;
            res.constant -= factor * eq3.constant;  // adjust the constant

            // subtract (factor * each term in eq3) from the result
            for (int k = 0; k < eq3.numVars; k++)
                res.addCoeff(eq3.varNames[k], -factor * eq3.coeffs[k]);

            // force the substituted variable's coefficient to exactly 0
            // (it should already be 0, but floating point math may leave tiny errors)
            int idx = res.findVar(var);
            if (idx != -1) res.coeffs[idx] = 0;

            cout << printEquation(res) << endl;
        }

        // -------------------------------------------------------
        // "D" or "D x2" - print Cramer's coefficient matrix
        //   "D"    -> prints the normal coefficient matrix
        //   "D x2" -> replaces the x2 column with the constants column
        //             (this is used in Cramer's rule to solve for x2)
        // -------------------------------------------------------
        else if (command == "D") {
            // read the rest of the line to check if a variable name was given
            string rest;
            getline(cin, rest);

            // remove leading spaces from rest to get the variable name (if any)
            string var = "";
            int s = 0;
            while (s < (int)rest.size() && rest[s] == ' ') s++;
            if (s < (int)rest.size()) var = rest.substr(s);

            // get all variable names (sorted alphabetically)
            string vars[MAXVARS];
            int sz = getAllVars(vars);

            // allocate the matrix on the heap: n rows, sz columns
            float **mat = new float*[n];
            for (int i = 0; i < n; i++) {
                mat[i] = new float[sz];
                for (int j = 0; j < sz; j++) {
                    // if a variable was specified AND this column matches it,
                    // replace it with the constant (right-hand side of equation)
                    if (!var.empty() && vars[j] == var)
                        mat[i][j] = equations[i].constant;
                    else
                        mat[i][j] = equations[i].getCoeff(vars[j]);
                }
            }

            // print the matrix row by row, with tabs between columns
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < sz; j++) {
                    float v = mat[i][j];
                    if (v == (int)v) cout << (int)v;  // print as integer if possible
                    else             cout << v;
                    if (j < sz - 1) cout << "\t";     // tab between columns, not after last
                }
                cout << endl;
            }

            // free the matrix memory (important! prevents memory leaks)
            for (int i = 0; i < n; i++) delete[] mat[i];
            delete[] mat;
        }

        // -------------------------------------------------------
        // "D_value" - print the determinant of the coefficient matrix
        // -------------------------------------------------------
        else if (command == "D_value") {
            string vars[MAXVARS];
            int sz = getAllVars(vars);

            // build the coefficient matrix on the heap
            float **mat = new float*[n];
            for (int i = 0; i < n; i++) {
                mat[i] = new float[sz];
                for (int j = 0; j < sz; j++)
                    mat[i][j] = equations[i].getCoeff(vars[j]);
            }

            float det = determinant(mat, sz);

            if (det == (int)det) cout << (int)det << endl;
            else                 cout << det << endl;

            // free memory
            for (int i = 0; i < n; i++) delete[] mat[i];
            delete[] mat;
        }

        // -------------------------------------------------------
        // "solve" - solve the system using Cramer's Rule
        //
        // Cramer's Rule:
        //   For each variable xi:
        //     value of xi = determinant(Di) / determinant(D)
        //   where D  = coefficient matrix
        //         Di = same matrix but with column i replaced by the constants
        //
        // If determinant(D) == 0, there is no unique solution.
        // -------------------------------------------------------
        else if (command == "solve") {
            string vars[MAXVARS];
            int sz = getAllVars(vars);

            // build the coefficient matrix
            float **mat = new float*[n];
            for (int i = 0; i < n; i++) {
                mat[i] = new float[sz];
                for (int j = 0; j < sz; j++)
                    mat[i][j] = equations[i].getCoeff(vars[j]);
            }

            float det = determinant(mat, sz);  // determinant of the coefficient matrix

            if (det == 0) {
                // if determinant is 0, the system has no unique solution
                cout << "No Solution" << endl;
            } else {
                // solve for each variable using Cramer's rule
                for (int k = 0; k < sz; k++) {
                    // build matrix Di: same as mat but column k replaced with constants
                    float **matk = new float*[n];
                    for (int i = 0; i < n; i++) {
                        matk[i] = new float[sz];
                        for (int j = 0; j < sz; j++)
                            matk[i][j] = (j == k) ? equations[i].constant : mat[i][j];
                    }

                    float detk = determinant(matk, sz);  // determinant of Di
                    float val  = detk / det;              // xi = det(Di) / det(D)

                    cout << vars[k] << "=";
                    if (val == (int)val) cout << (int)val;
                    else                 cout << val;
                    cout << endl;

                    // free matk memory
                    for (int i = 0; i < n; i++) delete[] matk[i];
                    delete[] matk;
                }
            }

            // free mat memory
            for (int i = 0; i < n; i++) delete[] mat[i];
            delete[] mat;
        }

    } // end while (reading commands)

    // Free the equations array that we allocated at the start
    // This is good practice: always free what you allocate with 'new'
    delete[] equations;

    return 0;  // 0 means the program ended successfully
}
