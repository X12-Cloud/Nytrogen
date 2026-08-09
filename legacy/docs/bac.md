```C++
// Any digit literal
        if (std::isdigit(currentChar)) {
            std::string value;
            int startColumn = column;
	    while (currentPos < sourceCode.length() && std::isdigit(sourceCode[currentPos])) {
    		value += sourceCode[currentPos++];
    		column++;
	    }

	    bool has_dot = false;
	    char suffix = value.back(); // last char of value

	    if (currentPos < sourceCode.length() && sourceCode[currentPos] == '.') {
    		has_dot = true;
    		value += sourceCode[currentPos++]; // Add the dot to the string
    		column++;

    		while (currentPos < sourceCode.length() && std::isdigit(sourceCode[currentPos])) {
        	    value += sourceCode[currentPos++];
        	    column++;
    		}
	    }

	    if (currentPos < sourceCode.length() && (sourceCode[currentPos] == 'f' || sourceCode[currentPos] == 'd')) {
    		suffix = sourceCode[currentPos];
    		value += sourceCode[currentPos++];
		column++;
	    }

	    if (has_dot == true) {
		switch (suffix) {
		    case 'f': tokens.push_back({Token::FLOAT_LITERAL, value, line, startColumn});
		    case 'd': tokens.push_back({Token::DOUBLE_LITERAL, value, line, startColumn});
		    default: tokens.push_back({Token::DOUBLE_LITERAL, value, line, startColumn});
		} 
	    } else if (has_dot == false) {
                tokens.push_back({Token::INTEGER_LITERAL, value, line, startColumn});
                continue;
		}
	    }
        }
```
