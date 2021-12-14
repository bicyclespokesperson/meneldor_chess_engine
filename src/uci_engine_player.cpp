#include "uci_engine_player.h"

Uci_engine_player::Uci_engine_player(std::string name, std::filesystem::path engine_path)
: Player(std::move(name)),
  m_engine_path(std::move(engine_path)),
  m_board{}
{
  MY_ASSERT(std::filesystem::exists(m_engine_path), "Engine executable does not exist");

  if (pipe(m_to_child.data()) != 0 || pipe(m_from_child.data()) != 0)
  {
    MY_ASSERT(false, "Failed to open pipe");
  }

  MY_ASSERT(m_to_child[0] > STDERR_FILENO && m_to_child[1] > STDERR_FILENO && m_from_child[0] > STDERR_FILENO &&
              m_from_child[1] > STDERR_FILENO,
            "Pipes initialized incorrectly");

  int pid{0};
  if ((pid = fork()) < 0)
  {
    // Failed to fork
    MY_ASSERT(false, "Failed to fork process");
  }

  if (pid == 0)
  {
    // We are the child
    fflush(nullptr); // Clear any pending data on all output streams
    if (dup2(m_to_child[0], STDIN_FILENO) < 0 || dup2(m_from_child[1], STDOUT_FILENO) < 0)
    {
      MY_ASSERT(false, "Failed to set stdin and stdout in child process");
    }

    close(m_to_child[0]);
    close(m_to_child[1]);
    close(m_from_child[0]);
    close(m_from_child[1]);

    std::cout << "Launching: " << m_engine_path << "\n";
    std::array<char const*, 2> args{m_engine_path.c_str(), nullptr};

    //NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast) Necessary for execv interface
    execv(m_engine_path.c_str(), const_cast<char**>(args.data()));
    MY_ASSERT(false, "Failed to fork engine process");
  }
  else
  {
    // We are the parent
    m_child_pid = pid;

    // Close the ends of the pipe we won't use
    close(m_to_child[0]);
    close(m_from_child[1]);

    if (init_engine_())
    {
      std::cout << "Engine initialized successfully\n";
    }
    else
    {
      std::cout << "Failed to initialize engine\n";
    }
  }
}

bool Uci_engine_player::init_engine_()
{
  auto engine_info = receive_message_();
  std::cout << "Engine: " << engine_info << "\n";

  send_message_("uci");
  auto engine_uci_info = receive_message_();
  std::cout << "Engine: " << engine_uci_info << "\n";

  // Options could be read and/or set here

  int tries{3};
  send_message_("isready");
  auto msg = receive_message_();
  while (msg.find("readyok") == std::string::npos && tries > 0)
  {
    --tries;
    if (tries > 0)
    {
      msg = receive_message_();
    }
  }

  if (tries == 0)
  {
    std::cout << "Initialization failed: " << msg << "\n";
    return false;
  }

  return true;
}

Uci_engine_player::~Uci_engine_player()
{
  terminate_engine_process_();
  close(m_to_child[1]);
  close(m_from_child[0]);
}

std::optional<std::string> Uci_engine_player::get_next_move(std::istream& /* in */, std::ostream& out)
{
  out << get_name() << " thinking:\n";
  static const std::string move_prefix{"bestmove "};
  send_message_(std::string{"position fen "} + m_board.to_fen());

  int const depth = 6;
  send_message_(std::string{"go depth "} + std::to_string(depth));

  size_t index{0};
  auto msg = receive_message_();
  while ((index = msg.find(move_prefix)) == std::string::npos)
  {
    out << msg << "\n";
    msg = receive_message_();
  }
  out << msg << "\n";

  index += move_prefix.size();
  constexpr int c_uci_move_length{5};
  auto const best_move = msg.substr(index, c_uci_move_length);
  return best_move;
}

void Uci_engine_player::notify(std::string const& move)
{
  m_board.try_move_uci(move);
}

bool Uci_engine_player::set_position(std::string_view fen)
{
  if (auto board = Board::from_fen(fen))
  {
    m_board = *board;
    return true;
  }
  return false;
}

void Uci_engine_player::reset()
{
  send_message_("ucinewgame");
}

bool Uci_engine_player::send_message_(std::string_view msg)
{
  if (write(m_to_child[1], msg.data(), msg.size()) != static_cast<int64_t>(msg.size()))
  {
    MY_ASSERT(false, "Failed to write message");
    return false;
  }

  if (!msg.empty() && msg[msg.size() - 1] != '\n')
  {
    static char const* terminator = "\n";
    if (write(m_to_child[1], terminator, 1) != 1)
    {
      MY_ASSERT(false, "Failed to write line terminator");
      return false;
    }
  }
  return true;
}

std::string Uci_engine_player::receive_message_()
{
  constexpr size_t c_buffer_size{1024};
  std::array<char, c_buffer_size> buffer{};
  int bytes_read{0};
  if ((bytes_read = read(m_from_child[0], buffer.data(), c_buffer_size)) <= 0)
  {
    // Failed to read message
    MY_ASSERT(false, "Failed to read message");
  }

  std::string msg{buffer.data(), static_cast<size_t>(bytes_read)};
  if (!msg.empty() && msg.back() == '\n')
  {
    msg.resize(msg.size() - 1);
  }

  return msg;
}

void Uci_engine_player::terminate_engine_process_()
{
  if (m_child_pid == 0)
  {
    return;
  }

  kill(m_child_pid, SIGTERM);

  int status{0};
  if (int corpse = waitpid(m_child_pid, &status, 0); corpse != m_child_pid)
  {
    MY_ASSERT(false, "Unexpected result when attempting to kill child process");
  }
  else
  {
    std::cout << get_name() << " exited successfully\n";
  }
}
