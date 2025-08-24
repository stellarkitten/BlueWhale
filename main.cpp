#include <chrono>
#include "chess.hpp"

using namespace chess;

constexpr PieceType pawn = PieceType::PAWN;
constexpr PieceType knight = PieceType::KNIGHT;
constexpr PieceType bishop = PieceType::BISHOP;
constexpr PieceType rook = PieceType::ROOK;
constexpr PieceType queen = PieceType::QUEEN;
constexpr PieceType king = PieceType::KING;

constexpr Color white = Color::WHITE;
constexpr Color black = Color::BLACK;

// Data: https://github.com/official-stockfish/Stockfish

constexpr int pawn_value = 167;
constexpr int knight_value = 818;
constexpr int bishop_value = 870;
constexpr int rook_value = 1328;
constexpr int queen_value = 2610;

constexpr int size = 64;

constexpr int pawn_table[size] = {0, 0, 0, 0, 0, 0, 0, 0, -3, -1, 10, 12, 16, 14, 2, -10, -9, -11, 0, 10, 16, 13, -1, -12, 2, -10, 0, 8, 12, 2, -4, -6, 12, 1, -4, -2, 3, -2, 1, 7, 15, 4, 6, 26, 11, 2, -3, 2, -4, -4, 6, 6, 14, 2, 8, -1, 0, 0, 0, 0, 0, 0, 0, 0};
constexpr int knight_table[size] = {-136, -78, -62, -47, -47, -62, -78, -136, -72, -48, -22, -4, -4, -22, -48, -72, -50, -22, -1, 20, 20, -1, -22, -50, -35, 3, 26, 38, 38, 26, 3, -35, -40, -2, 26, 45, 45, 26, -2, -40, -30, -11, 21, 35, 35, 21, -11, -30, -68, -38, -24, 24, 24, -24, -38, -68, -150, -86, -56, -22, -22, -56, -86, -150};
constexpr int bishop_table[size] = {-38, -12, -16, -12, -12, -16, -12, -38, -18, -2, 0, 2, 2, 0, -2, -18, -8, 7, -2, 10, 10, -2, 7, -8, -9, 2, 9, 20, 20, 9, 2, -9, -10, 10, 2, 16, 16, 2, 10, -10, -16, 4, 2, 6, 6, 2, 4, -16, -17, -12, 2, 0, 0, 2, -12, -17, -33, -14, -18, -16, -16, -18, -14, -33};
constexpr int rook_table[size] = {-20, -16, -12, -7, -7, -12, -16, -20, -16, -11, -4, 2, 2, -4, -11, -16, -10, -10, -2, -2, -2, -2, -10, -10, -10, -2, -6, 0, 0, -6, -2, -10, -16, -4, 2, -2, -2, 2, -4, -16, -8, 0, 0, 11, 11, 0, 0, -8, 1, 8, 18, 6, 6, 18, 8, 1, 0, -10, 9, 11, 11, 9, -10, 0};
constexpr int queen_table[size] = {-33, -31, -26, -11, -11, -26, -31, -33, -28, -13, -7, 4, 4, -7, -13, -28, -21, -6, 2, 5, 5, 2, -6, -21, -10, 1, 11, 16, 16, 11, 1, -10, -14, 4, 10, 13, 13, 10, 4, -14, -21, -4, -2, 4, 4, -2, -4, -21, -28, -10, -7, 0, 0, -7, -10, -28, -38, -27, -21, -18, -18, -21, -27, -38};
constexpr int king_table[size] = {136, 186, 178, 137, 137, 178, 186, 136, 166, 202, 184, 157, 157, 184, 202, 166, 142, 194, 169, 148, 148, 169, 194, 142, 134, 173, 155, 135, 135, 155, 173, 134, 125, 172, 152, 134, 134, 152, 172, 125, 108, 158, 132, 111, 111, 132, 158, 108, 68, 120, 90, 82, 82, 90, 120, 68, 35, 74, 59, 38, 38, 59, 74, 35};

constexpr int eval_limit = 31800;

long long nodes = 0;


// Helper for evaluation_function
static int flip_square(const int square) { return Square(square).flip().index(); }


static int evaluation_function (const Board &board)
{
    // Get white pieces
    Bitboard white_pawn = board.pieces(pawn, white);
    Bitboard white_knight = board.pieces(knight, white);
    Bitboard white_bishop = board.pieces(bishop, white);
    Bitboard white_rook = board.pieces(rook, white);
    Bitboard white_queen = board.pieces(queen, white);
    Bitboard white_king = board.pieces(king, white);

    // Get black pieces
    Bitboard black_pawn = board.pieces(pawn, black);
    Bitboard black_knight = board.pieces(knight, black);
    Bitboard black_bishop = board.pieces(bishop, black);
    Bitboard black_rook = board.pieces(rook, black);
    Bitboard black_queen = board.pieces(queen, black);
    Bitboard black_king = board.pieces(king, black);

    int evaluation = 0;

    // Evaluate white pieces
    while (!white_pawn.empty()) evaluation += pawn_value + pawn_table[white_pawn.pop()];
    while (!white_knight.empty()) evaluation += knight_value + knight_table[white_knight.pop()];
    while (!white_bishop.empty()) evaluation += bishop_value + bishop_table[white_bishop.pop()];
    while (!white_rook.empty()) evaluation += rook_value + rook_table[white_rook.pop()];
    while (!white_queen.empty()) evaluation += queen_value + queen_table[white_queen.pop()];
    while (!white_king.empty()) evaluation += king_table[white_king.pop()];

    // Evaluate black pieces
    while (!black_pawn.empty()) evaluation -= pawn_value + pawn_table[flip_square(black_pawn.pop())];
    while (!black_knight.empty()) evaluation -= knight_value + knight_table[flip_square(black_knight.pop())];
    while (!black_bishop.empty()) evaluation -= bishop_value + bishop_table[flip_square(black_bishop.pop())];
    while (!black_rook.empty()) evaluation -= rook_value + rook_table[flip_square(black_rook.pop())];
    while (!black_queen.empty()) evaluation -= queen_value + queen_table[flip_square(black_queen.pop())];
    while (!black_king.empty()) evaluation -= king_table[flip_square(black_king.pop())];

    return evaluation;
}


static int quiesce(int alpha, const int beta, Board& board)
{
    // Source: https://www.chessprogramming.org/Quiescence_Search

    ++ nodes;

    // If black to move, get negative best_value
    int best_value = evaluation_function(board) * (board.sideToMove() == white ? 1 : -1);

    if (best_value >= beta) return best_value;
    if (best_value > alpha) alpha = best_value;

    // Get captures
    Movelist captures;
    movegen::legalmoves<movegen::MoveGenType::CAPTURE>(captures, board);

    for (const Move& capture : captures)
    {
        board.makeMove(capture);
        const int score = -quiesce(-beta, -alpha, board);
        board.unmakeMove(capture);

        if (score >= beta) return score;
        if (score > best_value) best_value = score;
        if (score > alpha) alpha = score;
    }

    return best_value;
}


static int alpha_beta(int alpha, const int beta, const int depth_left, Board& board, std::vector<Move>& pv)
{
    // Source: https://www.chessprogramming.org/Alpha-Beta

    ++ nodes;

    // Quiesce if end of alpha beta
    if (depth_left == 0) return quiesce(alpha, beta, board);

    // 0 evaluation if 50-move rule or 3-fold repetition
    if (board.isHalfMoveDraw() || board.isRepetition(1)) return 0;

    // Get moves
    Movelist moves;
    movegen::legalmoves<movegen::MoveGenType::ALL>(moves, board);

    // eval_limit evaluation if checkmate or 0 evaluation if stalemate
    if (moves.empty()) return board.inCheck() ? -eval_limit : 0;

    int best_value = -eval_limit;

    for (const Move& move : moves)
    {
        board.makeMove(move);
        std::vector<Move> child_pv;
        const int score = -alpha_beta(-beta, -alpha, depth_left - 1, board, child_pv);
        board.unmakeMove(move);

        if (score > best_value)
        {
            best_value = score;
            if (score > alpha) alpha = score;
            pv = {move};
            pv.insert(pv.end(), child_pv.begin(), child_pv.end());
        }

        if (score >= beta) return best_value;
    }

    return best_value;
}


int main()
{
    std::string input;
    Board board = Board();

    while (std::getline(std::cin, input))
    {
        std::istringstream iss(input);
        std::string token;
        iss >> token;

        if (token == "go")
        {
            int depth = 0;
            std::vector<Move> pv;
            
            const std::chrono::time_point<std::chrono::system_clock> start_time = std::chrono::system_clock::now();
            nodes = 0;
            
            while (true)
            {
                ++ depth;

                const int score = alpha_beta(-eval_limit, eval_limit, depth, board, pv);

                const long long time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start_time).count();

                const long long nps = (time > 0 ? static_cast<long long>(nodes) * 1000 / time : 0);

                std::cout << "info depth " << depth
                          << " score cp " << score
                          << " time " << time
                          << " nodes " << nodes
                          << " nps " << nps
                          << " pv "; for (const Move& move : pv) std::cout << uci::moveToUci(move) << " ";
                std::cout << std::endl;
            }
        }

        else if (token == "position")
        {
            std::string sub;
            iss >> sub;

            if (sub == "startpos") board = Board();

            else if (sub == "fen")
            {
                std::string fen, part;
                for (int i = 0; i < 6 && iss >> part; ++ i) fen += (i > 0 ? " " : "") + part;
                board = Board(fen);
            }

            while (iss >> sub)
            {
                if (sub == "moves")
                {
                    std::string move_string;
                    while (iss >> move_string)
                    {
                        Move move = uci::uciToMove(board, move_string);
                        board.makeMove(move);
                    }
                }
            }
        }

        else if (token == "quit") break;

        else if (token == "uci")
        {
            std::cout << "id name BlueWhale-v1-0\n"
                      << "id author StellarKitten\n"
                      << "uciok\n";
        }

        else if (token == "ucinewgame") board = Board();

        else if (token == "isready") std::cout << "readyok\n";
    }
}
