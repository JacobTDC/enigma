#include <iostream>
#include <array>
#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "modules/rotors.h"
#include "modules/reflectors.h"

#include "modules/helptxt.h"

using namespace std;

vector<string> split(string str, char s) {
  vector<string> vect;
  string buf = "";
  for (char c : str) {
    if (c == s) {
      vect.push_back(buf);
      buf = "";
    } else
      buf += c;
  }
  vect.push_back(buf);
  return vect;
}

int sigin(array<int,26> map, int value, int offset) {
  value = (value + offset) % 26;
  int out = map[value];
  return (out - offset + 26) % 26;
}

int sigout(array<int,26> map, int value, int offset) {
  value = (value + offset) % 26;
  int out = distance(map.begin(), find(map.begin(), map.end(), value));
  return (out - offset + 26) % 26;
}

int main(int argc, char* argv[]) {
  array<array<int,26>,3> rotors;
  array<array<bool,26>,3> notches;
  array<int,3> offsets{0};
  array<int,26> reflector;
  array<array<int,2>,10> plugboard{0};
  int plugboardSize = 0;

  array<array<string,2>,3> rotorstrs = { RotorI, RotorII, RotorIII };
  string reflectstr = ReflectorB;

  bool usedp = false;
  bool usedw = false;
  bool usedu = false;
  bool useds = false;

  int c;
  while ((c = getopt (argc, argv, "hp:w:u:s:")) != -1) {
    switch (c) {
      case 'h':
        cout << help_txt;
        exit(0);
        break;
      case 'p':
        if (usedp) {
          fprintf(stderr, "%s: option -%c can only be specified once.\n", argv[0], c);
          exit(1);
        }
        usedp = true;
        if (strlen(optarg) != 3) {
          fprintf(stderr, "%s: argument for option -%c should be a three letter string.\n", argv[0], c);
          exit(1);
        }
        for (int i = 0; i < 3; i++) {
          if (!isalpha(optarg[i])) {
            fprintf(stderr, "%s: argument for option -%c should be a three letter string.\n", argv[0], c);
            exit(1);
          }
        }
        for (int i = 0; i < 3; i++)
          offsets[i] = toupper(optarg[i]) - 'A';
        break;
      case 'w':
        if (usedw) {
          fprintf(stderr, "%s: option -%c can only be specified once.\n", argv[0], c);
          exit(1);
        }
        usedw = true;
        {
          vector<string> rotorset = split(optarg, ',');
          if (rotorset.size() != 3) {
            fprintf(stderr, "%s: argument for option -%c should be a comma delimited string containing three values.\n", argv[0], c);
            exit(1);
          }
          for (int i = 0; i < 3; i++) {
            /* */if (rotorset[i] == "I") rotorstrs[i] = RotorI;
            else if (rotorset[i] == "II") rotorstrs[i] = RotorII;
            else if (rotorset[i] == "III") rotorstrs[i] = RotorIII;
            else if (rotorset[i] == "IV") rotorstrs[i] = RotorIV;
            else if (rotorset[i] == "V") rotorstrs[i] = RotorV;
            else {
              fprintf(stderr, "%s: unknown rotor name \"%s\".\n", argv[0], rotorset[i].c_str());
              exit(1);
            }
          }
        }
        break;
      case 'u':
        if (usedu) {
          fprintf(stderr, "%s: option -%c can only be specified once.\n", argv[0], c);
          exit(1);
        }
        usedu = true;
        /* */if (strcmp(optarg, "A") == 0) reflectstr = ReflectorA;
        else if (strcmp(optarg, "B") == 0) reflectstr = ReflectorB;
        else if (strcmp(optarg, "C") == 0) reflectstr = ReflectorC;
        else {
          fprintf(stderr, "%s: unknown reflector name \"%s\".\n", argv[0], optarg);
          exit(1);
        }
        break;
      case 's':
        if (useds) {
          fprintf(stderr, "%s: option -%c can only be specified once.\n", argv[0], c);
          exit(1);
        }
        useds = true;
        {
          vector<string> plugs = split(optarg, ',');
          array<bool,26> usedLetters{false};
          if (plugs.size() < 1 || plugs.size() > 10) {
            fprintf(stderr, "%s: argument for option -%c should be a comma delimited string containing up to ten pairs of letters.\n", argv[0], c);
            exit(1);
          }
          for (int i = 0; i < plugs.size(); i++) {
            string link = plugs[i];
            if (link.size() != 2 || !isalpha(link[0]) || !isalpha(link[1])) {
              fprintf(stderr, "%s: argument for option -%c should be a comma delimited string containing up to ten pairs of letters.\n", argv[0], c);
              exit(1);
            }
            for (int j = 0; j < 2; j++) {
              int plug = toupper(link[j]) - 'A';
              if (usedLetters[plug]) {
                fprintf(stderr, "%s: you cannot use a plug letter twice.\n", argv[0]);
                exit(1);
              }
              usedLetters[plug] = true;
              plugboard[i][j] = plug;
            }
            plugboardSize++;
          }
        }
        break;
      case '?':
        exit(1);
        break;
      default:
        fprintf(stderr, "%s: encountered unexpected behavior, this is (probably) not your fault.", argv[0]);
        abort();
    }
  }

  if (optind < argc) {
    fprintf(stderr, "%s: %s does not take any arguments.\n", argv[0], argv[0]);
    exit(0);
  }

  for (int a = 0; a < 3; a++) {
    for (int b = 0; b < 26; b++)
      rotors[a][b] = rotorstrs[a][0][b] - 'A';
    for (int b = 0; b < rotorstrs[a][1].length(); b++)
      notches[a][rotorstrs[a][1][b] - 'A'] = true;
  }

  for (int i = 0; i < 26; i++)
    reflector[i] = reflectstr[i] - 'A';

  while (cin) {
    char dc = cin.get();
    unsigned int di = dc;
    if (cin) {
      if (isalpha(dc)) {
        int key = toupper(dc) - 'A';

        bool rotate = false;
        for (int i = 2; i >= 0; i--) {
          if (i == 2 || rotate || (i > 0 && notches[i][offsets[i]])) {
            rotate = notches[i][offsets[i]];
            offsets[i] = (offsets[i] + 1) % 26;
          }
        }

        for (int i = 0; i < plugboardSize; i++) {
          if (plugboard[i][0] == key) {
            key = plugboard[i][1];
          } else if (plugboard[i][1] == key) {
            key = plugboard[i][0];
          }
        }
        for (int i = 2; i >= 0; i--)
          key = sigin(rotors[i], key, offsets[i]);
        key = reflector[key];
        for (int i = 0; i < 3; i++)
          key = sigout(rotors[i], key, offsets[i]);
        for (int i = 0; i < plugboardSize; i++) {
          if (plugboard[i][0] == key) {
            key = plugboard[i][1];
          } else if (plugboard[i][1] == key) {
            key = plugboard[i][0];
          }
        }

        char out = key + 'A';
        if (islower(dc))
          out = tolower(out);
        cout << out;
      } else
        cout << dc;
    }
  }

  return 0;
}
