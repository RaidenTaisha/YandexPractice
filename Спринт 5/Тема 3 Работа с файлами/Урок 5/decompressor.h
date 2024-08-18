#pragma once

#include <string>
#include <fstream>

// напишите эту функцию
inline bool DecodeRLE(const std::string& src_name,
                      const std::string& dst_name) {
  using namespace std;

  ifstream in(src_name, ios::in | ios::binary);
  if (!in.is_open())
    return false;

  ofstream out(dst_name, ios::out | ios::binary);
  if (!out.is_open())
    return false;

  do {
    int header = in.get();
    if (header == istream::traits_type::eof()) {
      break;
    }
    int block_type = (header & 1);
    int data_size = (header >> 1) + 1;

    char buff[1024];
    if (block_type == 0) {
      in.read(buff, data_size);
      out.write(buff, in.gcount());
      continue;
    }

    char ch = in.get();
    if (static_cast<int>(ch) == istream::traits_type::eof()) {
      break;
    }

    string y(data_size, ch);
    out.seekp(0, ios::end);
    out << y;
  } while (in);

  return true;
}
