#include <sstream>
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

// Data: https://arxiv.org/pdf/2009.04374

constexpr int pawn_value = 100;
constexpr int knight_value = 305;
constexpr int bishop_value = 333;
constexpr int rook_value = 563;
constexpr int queen_value = 950;

// Helper for tables
constexpr int size = 64;

// Data: https://github.com/official-stockfish/Stockfish
// Middlegame and endgame tables are averaged, then normalized to 0, then rounded to the nearest integer

constexpr int pawn_table[size] = { 0, 0, 0, 0, 0, 0, 0, 0, -6, -4, 7, 8, 13, 10, -2, -14, -12, -14, -3, 7, 13, 10, -4, -16, -1, -13, -3, 5, 9, -1, -8, -9, 8, -2, -8, -5, 0, -5, -2, 4, 12, 0, 3, 22, 8, -1, -6, -2, -7, -7, 2, 2, 11, -2, 5, -4, 0, 0, 0, 0, 0, 0, 0, 0 };
constexpr int knight_table[size] = { -109, -52, -35, -20, -20, -35, -52, -109, -45, -21, 4, 23, 23, 4, -21, -45, -24, 5, 26, 47, 47, 26, 5, -24, -8, 30, 53, 65, 65, 53, 30, -8, -13, 25, 53, 72, 72, 53, 25, -13, -3, 16, 48, 62, 62, 48, 16, -3, -41, -12, 3, 51, 51, 3, -12, -41, -124, -59, -29, 5, 5, -29, -59, -124 };
constexpr int bishop_table[size] = { -33, -7, -11, -7, -7, -11, -7, -33, -13, 4, 6, 7, 7, 6, 4, -13, -3, 12, 3, 15, 15, 3, 12, -3, -4, 7, 14, 25, 25, 14, 7, -4, -5, 15, 8, 22, 22, 8, 15, -5, -11, 9, 7, 11, 11, 7, 9, -11, -12, -7, 7, 6, 6, 7, -7, -12, -28, -9, -13, -11, -11, -13, -9, -28 };
constexpr int rook_table[size] = { -17, -13, -9, -4, -4, -9, -13, -17, -13, -8, -1, 5, 5, -1, -8, -13, -6, -6, 2, 2, 2, 2, -6, -6, -6, 1, -3, 4, 4, -3, 1, -6, -13, 0, 5, 2, 2, 5, 0, -13, -5, 3, 3, 14, 14, 3, 3, -5, 4, 12, 21, 10, 10, 21, 12, 4, 4, -6, 12, 14, 14, 12, -6, 4 };
constexpr int queen_table[size] = { -23, -21, -16, -1, -1, -16, -21, -23, -19, -3, 3, 14, 14, 3, -3, -19, -11, 4, 12, 15, 15, 12, 4, -11, 0, 11, 21, 26, 26, 21, 11, 0, -5, 14, 20, 23, 23, 20, 14, -5, -11, 6, 7, 14, 14, 7, 6, -11, -18, -1, 3, 10, 10, 3, -1, -18, -28, -17, -11, -8, -8, -11, -17, -28 };
constexpr int king_table[size] = { 3, 53, 45, 4, 4, 45, 53, 3, 33, 69, 51, 24, 24, 51, 69, 33, 9, 61, 36, 15, 15, 36, 61, 9, 1, 40, 22, 2, 2, 22, 40, 1, -8, 40, 19, 2, 2, 19, 40, -8, -25, 26, 0, -22, -22, 0, 26, -25, -65, -12, -42, -51, -51, -42, -12, -65, -98, -59, -74, -94, -94, -74, -59, -98 };

constexpr int eval_limit = 31800;
long long nodes = 0;


static int flip_square(const int square)
{
    // Helper for evaluate
    return Square(square).flip().index();
}


static int evaluate(const Board& board)
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
    int best_value = evaluate(board) * (board.sideToMove() == white ? 1 : -1);

    if (best_value >= beta) return best_value;
    if (best_value > alpha) alpha = best_value;

    // Get captures
    Movelist captures;
    movegen::legalmoves<movegen::MoveGenType::CAPTURE>(captures, board);

    for (const Move& i : captures)
    {
        board.makeMove(i);
        const int score = -quiesce(-beta, -alpha, board);
        board.unmakeMove(i);

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

    // Order captures before quiet moves
    std::stable_partition(moves.begin(), moves.end(), [&](const Move& i) { return board.isCapture(i); });

    // Order pv before moves
    if (!pv.empty())
    {
        const Move pv_move = pv[0];
        const Movelist::iterator it = std::find(moves.begin(), moves.end(), pv_move);
        if (it != moves.begin() && it != moves.end()) std::swap(moves[0], *it);
    }

    int best_value = -eval_limit;

    for (const Move& i : moves)
    {
        std::vector<Move> child_pv;
        board.makeMove(i);
        const int score = -alpha_beta(-beta, -alpha, depth_left - 1, board, child_pv);
        board.unmakeMove(i);

        if (score >= beta) return score;
        if (score > best_value)
        {
            best_value = score;
            if (score > alpha)
            {
                alpha = score;
                pv = { i };
                pv.insert(pv.end(), child_pv.begin(), child_pv.end());
            }
        }
    }

    return best_value;
}


int main()
{
    Board board = Board();
    std::string input;

    while (std::getline(std::cin, input))
    {
        std::istringstream iss(input);
        std::string command;
        iss >> command;

        if (command == "go")
        {
            int depth = 0;
            std::vector<Move> pv;
            const std::chrono::time_point<std::chrono::steady_clock> start_time = std::chrono::steady_clock::now();
            nodes = 0;
            
            while (true)
            {
                ++ depth;

                const int score = alpha_beta(-eval_limit, eval_limit, depth, board, pv);
                const long long time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time).count();

                // Multiply by 1000 to convert millisecond to second
                const long long nps = (time > 0 ? nodes * 1000 / time : 0);

                std::cout << "info depth " << depth
                          << " score cp " << score
                          << " time " << time
                          << " nodes " << nodes
                          << " nps " << nps
                          << " pv ";
                for (const Move& i : pv) std::cout << uci::moveToUci(i) << " ";
                std::cout << std::endl;
            }
        }

        else if (command == "position")
        {
            std::string subcommand;
            std::string argument;
            iss >> subcommand;

            if (subcommand == "startpos")
            {
                board = Board();

                // Remove "moves" from argument
                iss >> argument;
            }

            else if (subcommand == "fen")
            {
                std::string fen;
                while (iss >> argument && argument != "moves") fen += argument + " ";
                board = Board(fen);
            }

            std::vector<std::string> moves;
            while (iss >> argument) moves.push_back(argument);

            for (const std::string& i : moves)
            {
                Move move = uci::uciToMove(board, i);
                board.makeMove(move);
            }
        }

        else if (command == "quit") break;

        else if (command == "uci")
        {
            std::cout << "id name BlueWhale-v1-4\n"
                      << "id author StellarKitten\n"
                      << "uciok\n";
        }

        else if (command == "ucinewgame") board = Board();

        else if (command == "isready") std::cout << "readyok\n";
    }
}
