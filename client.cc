// Copyright 2024 Chris Taks

#include <proj3/client.h>

#include <iostream>
#include <vector>
#include <string>

void DomainSocketClient::Run(int argc, char *argv[]) {
  std::cout << "Client initializing..." << std::endl;
  if (!Init()) {
    exit(1);
  }

  std::cout << "Client connecting..." << std::endl;
  if (!Connect()) {
    exit(2);
  }
  std::cout << "SERVER CONNECTION ACCEPTED" << std::endl;
  // send the data
  std::string finalInput;
  for (int i = 2; i < argc; ++i) {
    std::string input(argv[i]);
    finalInput += input;
    finalInput += kUS;
  }
  finalInput += kEoT;
  ::size_t bytes_wrote = Write(finalInput);

  if (bytes_wrote < 0) {
    std::cerr << "Client terminating..." << std::endl;
    exit(3);
  } else if (bytes_wrote == 0) {
    std::cerr << "Server disconnected" << std::endl;
    exit(4);
  }

  // recieve the data
  std::string msg;

  ::ssize_t bytes_read = Read(&msg);
  std::cout << "BYTES RECEIVED: " << bytes_read << std::endl;
  if (bytes_read < 0) {
    std::cerr << "Server shutting down..." << std::endl;
    exit(0);
  } else if (bytes_read) {
    Close(socket_fd_);
  }

  ::size_t found = msg.find("INVALID");
  if (found != std::string::npos) {
    std::cout << "ERROR: " << msg << std::endl;
    exit(1);
  }

  // print/process the data
  std::vector<std::string> theLines;
  std::string toBeAdded;
  for (size_t i = 0; i < msg.size(); ++i) {
    if ((msg[i] == kUS || msg[i] == kEoT) && toBeAdded.size() > 0) {
      theLines.push_back(toBeAdded);
      toBeAdded = "";
    } else {
      toBeAdded += msg[i];
    }
  }

  // sort line numbers
  std::vector<int> lineNumbers;
  for (int i = 3; i < argc; ++i) {
    lineNumbers.push_back(std::stoi(argv[i]));
  }
  std::sort(lineNumbers.begin(), lineNumbers.end());

  int lineNumber = 0;
  for (std::string line : theLines) {
    std::string finishedLine = "line ";
    finishedLine += std::to_string(lineNumbers[lineNumber]);
    finishedLine += ": ";
    finishedLine += line;
    finishedLine += " = ";
    finishedLine += processEquation(line);
    std::cout << finishedLine << std::endl;
    ++lineNumber;
  }
}

double DomainSocketClient::AddNumbers(double a, double b) {
  return a + b;
}

double DomainSocketClient::SubtractNumbers(double a, double b) {
  return a - b;
}

double DomainSocketClient::MultiplyNumbers(double a, double b) {
  return a * b;
}

double DomainSocketClient::DivideNumbers(double a, double b) {
  return a / b;
}

bool DomainSocketClient::IsOperator(std::string arg) {
  return arg == "+" || arg == "-" || arg == "x" || arg == "/";
}

std::string DomainSocketClient::processEquation(std::string line) {
  std::vector<double> numbers;
  std::vector<std::string> operators;
  // break up the string
  std::vector<std::string> args;
  std::string component;
  for (char c : line) {
    if (c != ' ') {
      component += c;
    } else {
      args.push_back(component);
      component = "";
    }
  }
  args.push_back(component);
  // adding the args to their respective vectors
  for (size_t i = 0; i < args.size(); ++i) {
    if (IsOperator(args[i])) {
      operators.push_back(args[i]);
      if (operators.back() == "x" || operators.back() == "/") {
        double a = numbers.back();
        numbers.pop_back();
        if (operators.back() == "x") {
          numbers.push_back(MultiplyNumbers(a, std::stod(args[i+1])));
        }
        if (operators.back() == "/") {
          numbers.push_back(DivideNumbers(a, std::stod(args[i+1])));
        }
        ++i;  // iterate past the next variable since it was already used
      }
    } else {
      numbers.push_back(std::stod(args[i]));
    }
  }
  // next, just add / subtract the remaining numbers
  for (std::string& op : operators) {
    if (op == "+") {
      numbers[0] = AddNumbers(numbers[0], numbers[1]);
      numbers.erase(numbers.begin()+1);
    }
    if (op == "-") {
      numbers[0] = SubtractNumbers(numbers[0], numbers[1]);
      numbers.erase(numbers.begin()+1);
    }
  }
  return std::to_string(static_cast<int>(numbers[0]));
}

int main(int argc, char *argv[]) {
  DomainSocketClient dsc(argv[1]);
  dsc.Run(argc, argv);

  return 0;
}
