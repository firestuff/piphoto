#include <arpa/inet.h>
#include <assert.h>
#include <fcntl.h>
#include <png.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <array>
#include <experimental/string_view>
#include <iostream>
#include <string>

namespace std {
using string_view = experimental::string_view;
}

std::string ReadFile(const std::string& filename);
void WriteFile(const std::string& filename, const std::string& contents);


struct Color {
  // 32-bit for compiler convenience, but values are 16-bit
  uint32_t r;
  uint32_t g;
  uint32_t b;

  uint32_t Difference(const Color& other) const;
};

uint32_t Color::Difference(const Color& other) const {
  return (
    ((r > other.r) ? (r - other.r) : (other.r - r)) +
    ((g > other.g) ? (g - other.g) : (other.g - g)) +
    ((b > other.b) ? (b - other.b) : (other.b - b))
  );
}


constexpr uint32_t kNumColorChecker = 24;
constexpr std::array<Color, kNumColorChecker> kColorCheckerSrgb = {{
  {0x7300, 0x5200, 0x4400},
  {0xc200, 0x9600, 0x8200},
  {0x6200, 0x7a00, 0x9d00},
  {0x5700, 0x6c00, 0x4300},
  {0x8500, 0x8000, 0xb100},
  {0x6700, 0xbd00, 0xaa00},
  {0xd600, 0x7e00, 0x2c00},
  {0x5000, 0x5b00, 0xa600},
  {0xc100, 0x5a00, 0x6300},
  {0x5e00, 0x3c00, 0x6c00},
  {0x9d00, 0xbc00, 0x4000},
  {0xe000, 0xa300, 0x2e00},
  {0x3800, 0x3d00, 0x9600},
  {0x4600, 0x9400, 0x4900},
  {0xaf00, 0x3600, 0x3c00},
  {0xe700, 0xc700, 0x1f00},
  {0xbb00, 0x5600, 0x9500},
  {0x0800, 0x8500, 0xa100},
  {0xf300, 0xf300, 0xf200},
  {0xc800, 0xc800, 0xc800},
  {0xa000, 0xa000, 0xa000},
  {0x7a00, 0x7a00, 0x7900},
  {0x5500, 0x5500, 0x5500},
  {0x3400, 0x3400, 0x3400},
}};


struct Coord {
  uint32_t x;
  uint32_t y;
};

std::ostream& operator<<(std::ostream& os, const Coord& coord);

std::ostream& operator<<(std::ostream& os, const Coord& coord) {
  return os << "(" << coord.x << ", " << coord.y << ")";
}

template <uint32_t X, uint32_t Y>
struct Image : public std::array<std::array<Color, X>, Y> {
  std::array<Coord, kNumColorChecker> ColorCheckerClosest() const;
};

template <uint32_t X, uint32_t Y>
std::array<Coord, kNumColorChecker> Image<X, Y>::ColorCheckerClosest() const {
  std::array<Coord, kNumColorChecker> closest;
  std::array<uint32_t, kNumColorChecker> diff;
  diff.fill(UINT32_MAX);

  for (uint32_t y = 0; y < Y; ++y) {
    const auto& row = this->at(y);

    for (uint32_t x = 0; x < X; ++x) {
      const auto& pixel = row.at(x);

      for (uint32_t cc = 0; cc < kNumColorChecker; ++cc) {
        auto pixel_diff = pixel.Difference(kColorCheckerSrgb.at(cc));
        if (pixel_diff < diff.at(cc)) {
          diff.at(cc) = pixel_diff;
          closest.at(cc) = {x, y};
        }
      }
    }
  }

  return closest;
}


template <uint32_t X, uint32_t Y, uint32_t D, uint32_t A, uint32_t P>
class PiRaw {
 public:
  PiRaw(std::unique_ptr<Image<X, Y>>);
  static PiRaw FromJpeg(const std::string_view& jpeg);
  static PiRaw FromRaw(const std::string_view& raw);

  std::string ToPng();

  const Image<X, Y>& GetImage();

 private:
  static constexpr uint32_t kJpegHeaderBytes = 32768;
  static constexpr const char* kJpegHeaderMagic = "BRCM";
  static constexpr uint32_t kPixelsPerChunk = 4;
  static constexpr uint32_t kBitsPerByte = 8;

  static constexpr uint32_t GetRawBytes();
  static constexpr uint32_t GetRowBytes();
  static constexpr uint32_t GetNumRows();
  static constexpr uint32_t GetChunkBytes();

  static constexpr uint32_t Align(uint32_t val);

  typedef std::array<uint32_t, kPixelsPerChunk> Chunk;

  static Chunk GetChunk(const std::string_view& raw, const uint32_t x_chunk, const uint32_t y);
  static Color CombineRaw(uint32_t y0x0, uint32_t y0x1, uint32_t y1x0, uint32_t y1x1);

  std::unique_ptr<Image<X, Y>> image_;
};

typedef PiRaw<3280,2464,10,16,2> PiRaw2;

template <uint32_t X, uint32_t Y, uint32_t D, uint32_t A, uint32_t P>
PiRaw<X,Y,D,A,P>::PiRaw(std::unique_ptr<Image<X, Y>> image)
    : image_(std::move(image)) {}

template <uint32_t X, uint32_t Y, uint32_t D, uint32_t A, uint32_t P>
PiRaw<X,Y,D,A,P> PiRaw<X,Y,D,A,P>::FromJpeg(const std::string_view& jpeg) {
  auto container_len = GetRawBytes() + kJpegHeaderBytes;
  assert(jpeg.substr(jpeg.size() - container_len, 4) == kJpegHeaderMagic);
  return FromRaw(jpeg.substr(jpeg.size() - GetRawBytes(), GetRawBytes()));
}

template <uint32_t X, uint32_t Y, uint32_t D, uint32_t A, uint32_t P>
PiRaw<X,Y,D,A,P> PiRaw<X,Y,D,A,P>::FromRaw(const std::string_view& raw) {
  static_assert(X % 2 == 0);
  static_assert(Y % 2 == 0);
  static_assert(kPixelsPerChunk == 4);

  assert(raw.size() == GetRawBytes());

  auto image = std::make_unique<Image<X, Y>>();

  for (uint32_t y = 0, out_y = 0; y < Y; y += 2, ++out_y) {
    for (uint32_t x_chunk = 0, out_x = 0; x_chunk < X / kPixelsPerChunk; ++x_chunk, out_x += kPixelsPerChunk / 2) {
      auto chunk1 = GetChunk(raw, x_chunk, y + 0);
      auto chunk2 = GetChunk(raw, x_chunk, y + 1);
      image->at(out_y).at(out_x + 0) = CombineRaw(chunk1.at(0), chunk1.at(1), chunk2.at(0), chunk2.at(1));
      image->at(out_y).at(out_x + 1) = CombineRaw(chunk1.at(2), chunk1.at(3), chunk2.at(2), chunk2.at(3));
    }
  }
  return PiRaw<X,Y,D,A,P>(std::move(image));
}

template <uint32_t X, uint32_t Y, uint32_t D, uint32_t A, uint32_t P>
constexpr uint32_t PiRaw<X,Y,D,A,P>::GetRawBytes() {
  return GetRowBytes() * GetNumRows();
}

template <uint32_t X, uint32_t Y, uint32_t D, uint32_t A, uint32_t P>
constexpr uint32_t PiRaw<X,Y,D,A,P>::GetRowBytes() {
  return Align(Align(X + P) * D / kBitsPerByte);
}

template <uint32_t X, uint32_t Y, uint32_t D, uint32_t A, uint32_t P>
constexpr uint32_t PiRaw<X,Y,D,A,P>::GetNumRows() {
  return Align(Y + P);
}

template <uint32_t X, uint32_t Y, uint32_t D, uint32_t A, uint32_t P>
constexpr uint32_t PiRaw<X,Y,D,A,P>::GetChunkBytes() {
  return D * kPixelsPerChunk / kBitsPerByte;
}

template <uint32_t X, uint32_t Y, uint32_t D, uint32_t A, uint32_t P>
constexpr uint32_t PiRaw<X,Y,D,A,P>::Align(uint32_t val) {
  return (~(A - 1)) & ((val) + (A - 1));
}

template <uint32_t X, uint32_t Y, uint32_t D, uint32_t A, uint32_t P>
typename PiRaw<X,Y,D,A,P>::Chunk PiRaw<X,Y,D,A,P>::GetChunk(const std::string_view& raw, const uint32_t x_chunk, const uint32_t y) {
  // Function is bit depth & layout specific
  static_assert(D == 10);

  auto start = y * GetRowBytes() + x_chunk * GetChunkBytes();
  auto high0 = static_cast<uint32_t>(raw.at(start + 0));
  auto high1 = static_cast<uint32_t>(raw.at(start + 1));
  auto high2 = static_cast<uint32_t>(raw.at(start + 2));
  auto high3 = static_cast<uint32_t>(raw.at(start + 3));
  auto packed_low = static_cast<uint32_t>(raw.at(start + 4));

  Chunk ret;
  ret.at(0) = ((high0 << 2) | ((packed_low >> 6) & 0b11)) << 6;
  ret.at(1) = ((high1 << 2) | ((packed_low >> 4) & 0b11)) << 6;
  ret.at(2) = ((high2 << 2) | ((packed_low >> 2) & 0b11)) << 6;
  ret.at(3) = ((high3 << 2) | ((packed_low >> 0) & 0b11)) << 6;
  return ret;
}

template <uint32_t X, uint32_t Y, uint32_t D, uint32_t A, uint32_t P>
Color PiRaw<X,Y,D,A,P>::CombineRaw(uint32_t y0x0, uint32_t y0x1, uint32_t y1x0, uint32_t y1x1) {
  // Function is bit layout specific
  Color ret;
  ret.r = y1x1;
  ret.g = (y0x1 + y1x0) / 2;
  ret.b = y0x0;
  return ret;
}

static void WriteCallback(png_structp png_ptr, png_bytep data, png_size_t length) {
  auto dest = static_cast<std::string*>(png_get_io_ptr(png_ptr));
  dest->append(reinterpret_cast<char*>(data), length);
}

template <uint32_t X, uint32_t Y, uint32_t D, uint32_t A, uint32_t P>
std::string PiRaw<X,Y,D,A,P>::ToPng() {
  std::string ret;

  auto png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  assert(png_ptr);
  auto info_ptr = png_create_info_struct(png_ptr);
  assert(info_ptr);

  png_set_write_fn(png_ptr, &ret, &WriteCallback, nullptr);
  png_set_IHDR(png_ptr, info_ptr, X / 2, Y / 2,
    16, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
    PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

  png_write_info(png_ptr, info_ptr);
  for (auto& row : *image_) {
    std::array<uint16_t, X * 3> out_row;
    for (uint32_t x = 0; x < X; ++x) {
      out_row[x * 3 + 0] = htons(static_cast<uint16_t>(row[x].r));
      out_row[x * 3 + 1] = htons(static_cast<uint16_t>(row[x].g));
      out_row[x * 3 + 2] = htons(static_cast<uint16_t>(row[x].b));
    }
    png_write_row(png_ptr, reinterpret_cast<unsigned char*>(out_row.data()));
  }
  png_write_end(png_ptr, nullptr);

  png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
  png_destroy_write_struct(&png_ptr, &info_ptr);

  return ret;
}

template <uint32_t X, uint32_t Y, uint32_t D, uint32_t A, uint32_t P>
const Image<X, Y>& PiRaw<X,Y,D,A,P>::GetImage() {
  return *image_;
}

std::string ReadFile(const std::string& filename) {
  int fh = open(filename.c_str(), O_RDONLY);
  assert(fh != -1);

  struct stat st;
  assert(fstat(fh, &st) == 0);
  
  std::string contents;
  contents.resize(static_cast<size_t>(st.st_size));

  assert(read(fh, &contents[0], static_cast<size_t>(st.st_size)) == st.st_size);
  assert(close(fh) == 0);

  return contents;
}

void WriteFile(const std::string& filename, const std::string& contents) {
  int fh = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
  assert(fh != -1);
  assert(write(fh, &contents[0], contents.size()) == static_cast<ssize_t>(contents.size()));
  assert(close(fh) == 0);
}


int main() {
  auto raw = PiRaw2::FromJpeg(ReadFile("test.jpg"));
  auto closest = raw.GetImage().ColorCheckerClosest();
  for (uint32_t cc = 0; cc < kNumColorChecker; ++cc) {
    std::cout << cc << ": " << closest.at(cc) << std::endl;
  }
  WriteFile("test.png", raw.ToPng());
}
