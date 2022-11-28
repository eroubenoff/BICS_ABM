#include<vector>
#include<string>
#include<sstream> 
#include<iostream>
using namespace std;


vector<string> get_csv_row(istream &fin, int expected_length = -1, char sep = ',') {
    // Read the Data from the file
    // as String Vector
    vector<string> row;
    string temp, word;
    char c; 

    // Get the first row as header
    getline(fin, temp);


    // Convert to a stringstream
    stringstream s(temp);

    // Boolean to keep track if the word is quoted
    bool enquo = false;
    while (s >> c) {
        // Set quote flag 
        if (c == '\"') {

            // Set flag as needed
            if (!enquo) {
                enquo = true;
            } 
            else if (enquo) {
                enquo = false;
            }

            // Append to word and read next char
            // word += c;
            continue; 

        } 

        // If we're reading the seperator and not quoted,
        // push back the word to row and reset word
        else if ((c == sep ) & (!enquo) ) {

            // If there is no data (i.e., ""), push back a -1 
            if (word == ""){
                word= "-1";
            }

            // Push word to row
            row.push_back(word);

            // Reset word
            word = "";

            continue;
            
        } 

        // Else, read append the character to word
        else {
            word += c;
        }


    }

    if (word != "") {
        row.push_back(word);
    }

    if ((expected_length != -1 ) & (row.size() != expected_length)) {
        row.resize(expected_length, "NA");
    }

    return row;

}
