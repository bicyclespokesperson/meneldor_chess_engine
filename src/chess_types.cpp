#include "chess_types.h"
Piece from_char(char c)
{
  switch (c)
  {
    case 'N':
      return Piece::knight;
    case 'B':
      return Piece::bishop;
    case 'R':
      return Piece::rook;
    case 'Q':
      return Piece::queen;
    case 'K':
      return Piece::king;
    case 'P':
      return Piece::pawn;
    default:
      return Piece::empty;
  }
}

bool Threefold_repetition_detector::add_fen(std::string_view fen)
{
  // Move counts don't matter for threefold repetition, so find the end
  // of the fen string that doesn't include those
  auto index = fen.find_last_not_of(' ');
  index = fen.find_last_of(' ', index);
  index = fen.find_last_not_of(' ', index);
  index = fen.find_last_of(' ', index);
  index = fen.find_last_not_of(' ', index);

  std::string abridged_fen{fen.substr(0, index)};

  if ((++m_previous_positions[abridged_fen]) >= 3)
  {
    m_is_drawn = true;
  }
  return m_is_drawn;
}

bool Threefold_repetition_detector::is_drawn() const
{
  return m_is_drawn;
}

std::ostream& operator<<(std::ostream& os, Piece const& self)
{
  switch (self)
  {
    case Piece::pawn:
      os << 'P';
      break;
    case Piece::bishop:
      os << 'B';
      break;
    case Piece::knight:
      os << 'N';
      break;
    case Piece::rook:
      os << 'R';
      break;
    case Piece::queen:
      os << 'Q';
      break;
    case Piece::king:
      os << 'K';
      break;
    case Piece::empty:
      os << '_';
      break;
    default:
      os << '-';
      break;
  }

  return os;
}

std::ostream& operator<<(std::ostream& os, Color const& self)
{
  os << ((self == Color::white) ? std::string{"white"} : std::string{"black"});
  return os;
}
