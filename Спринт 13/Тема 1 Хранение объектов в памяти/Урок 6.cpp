#include <array>
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>

const static int N = 4;
const std::array<char, N> Symbols = {'A', 'T', 'G', 'C'};

struct Nucleotide {
  char symbol;
  size_t position;
  int chromosome_num;
  int gene_num;
  bool is_marked;
  char service_info;
};

struct CompactNucleotide {
  uint32_t position;
  uint32_t gene_num :15;
  uint32_t is_marked :1;
  uint32_t service_info: 8;
  uint32_t chromosome_num: 6;
  uint32_t symbol: 2;
};

CompactNucleotide Compress(const Nucleotide& n) {
  CompactNucleotide result;

  result.position = static_cast<uint32_t>(n.position);
  result.gene_num = static_cast<uint32_t>(n.gene_num);
  result.is_marked = static_cast<uint32_t>(n.is_marked);
  result.service_info = static_cast<uint32_t>(n.service_info);
  result.chromosome_num = static_cast<uint32_t>(n.chromosome_num);
  result.symbol = static_cast<uint32_t>(std::distance(Symbols.begin(),
                                                      std::find(Symbols.begin(), Symbols.end(), n.symbol)));

  return result;
}

Nucleotide Decompress(const CompactNucleotide& cn) {
  Nucleotide result;

  result.symbol = Symbols.at(cn.symbol);
  result.position = cn.position;
  result.chromosome_num = static_cast<int>(cn.chromosome_num);
  result.gene_num = static_cast<int>(cn.gene_num);
  result.is_marked = static_cast<int>(cn.is_marked) == 1;
  result.service_info = static_cast<char>(cn.service_info);

  return result;
}

static_assert(sizeof(CompactNucleotide) <= 8, "Your CompactNucleotide is not compact enough");
static_assert(alignof(CompactNucleotide) == 4, "Don't use '#pragma pack'!");
bool operator==(const Nucleotide& lhs, const Nucleotide& rhs) {
  return (lhs.symbol == rhs.symbol) && (lhs.position == rhs.position) && (lhs.chromosome_num == rhs.chromosome_num)
      && (lhs.gene_num == rhs.gene_num) && (lhs.is_marked == rhs.is_marked) && (lhs.service_info == rhs.service_info);
}
void TestSize() {
  assert(sizeof(CompactNucleotide) <= 8);
}
void TestCompressDecompress() {
  Nucleotide source;
  source.symbol = 'T';
  source.position = 1'000'000'000;
  source.chromosome_num = 48;
  source.gene_num = 1'000;
  source.is_marked = true;
  source.service_info = '!';

  CompactNucleotide compressedSource = Compress(source);
  Nucleotide decompressedSource = Decompress(compressedSource);

  assert(source == decompressedSource);
}

int main() {
  TestSize();
  TestCompressDecompress();
}
