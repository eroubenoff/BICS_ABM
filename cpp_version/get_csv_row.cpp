#include<vector>
#include<string>
#include<sstream> 
using std::string;
using std::vector;
using std::stringstream;
using std::istream;


vector<string> get_csv_row(istream &fin) {
    // Read the Data from the file
    // as String Vector
    vector<string> row;
    string temp, word;

    // Get the first row as header
    getline(fin, temp);

    // Convert to a stringstream
    stringstream s(temp);

    // Loop through each word and append to vector
    while (getline(s, word, ',')) {
        row.push_back(word);
    } 

    return row;

}