#include <sstream>
#include <chrono>
#include "chess.hpp"

using namespace chess;

constexpr PieceType piece_types[] = { PieceType::PAWN, PieceType::KNIGHT, PieceType::BISHOP, PieceType::ROOK, PieceType::QUEEN, PieceType::KING };

constexpr Color white = Color::WHITE;
constexpr Color black = Color::BLACK;

constexpr GameResultReason checkmate = GameResultReason::CHECKMATE;

// Data: https://arxiv.org/pdf/2009.04374
constexpr int piece_value[] = { 100, 305, 333, 563, 950, 0 };

// Data: https://github.com/official-stockfish/Stockfish
// Middlegame and endgame tables are averaged, then normalized to 0, then rounded to the nearest integer
constexpr int piece_table[6][64] =
{
    { 0, 0, 0, 0, 0, 0, 0, 0, -6, -4, 7, 8, 13, 10, -2, -14, -12, -14, -3, 7, 13, 10, -4, -16, -1, -13, -3, 5, 9, -1, -8, -9, 8, -2, -8, -5, 0, -5, -2, 4, 12, 0, 3, 22, 8, -1, -6, -2, -7, -7, 2, 2, 11, -2, 5, -4, 0, 0, 0, 0, 0, 0, 0, 0 },
    { -109, -52, -35, -20, -20, -35, -52, -109, -45, -21, 4, 23, 23, 4, -21, -45, -24, 5, 26, 47, 47, 26, 5, -24, -8, 30, 53, 65, 65, 53, 30, -8, -13, 25, 53, 72, 72, 53, 25, -13, -3, 16, 48, 62, 62, 48, 16, -3, -41, -12, 3, 51, 51, 3, -12, -41, -124, -59, -29, 5, 5, -29, -59, -124 },
    { -33, -7, -11, -7, -7, -11, -7, -33, -13, 4, 6, 7, 7, 6, 4, -13, -3, 12, 3, 15, 15, 3, 12, -3, -4, 7, 14, 25, 25, 14, 7, -4, -5, 15, 8, 22, 22, 8, 15, -5, -11, 9, 7, 11, 11, 7, 9, -11, -12, -7, 7, 6, 6, 7, -7, -12, -28, -9, -13, -11, -11, -13, -9, -28 },
    { -17, -13, -9, -4, -4, -9, -13, -17, -13, -8, -1, 5, 5, -1, -8, -13, -6, -6, 2, 2, 2, 2, -6, -6, -6, 1, -3, 4, 4, -3, 1, -6, -13, 0, 5, 2, 2, 5, 0, -13, -5, 3, 3, 14, 14, 3, 3, -5, 4, 12, 21, 10, 10, 21, 12, 4, 4, -6, 12, 14, 14, 12, -6, 4 },
    { -23, -21, -16, -1, -1, -16, -21, -23, -19, -3, 3, 14, 14, 3, -3, -19, -11, 4, 12, 15, 15, 12, 4, -11, 0, 11, 21, 26, 26, 21, 11, 0, -5, 14, 20, 23, 23, 20, 14, -5, -11, 6, 7, 14, 14, 7, 6, -11, -18, -1, 3, 10, 10, 3, -1, -18, -28, -17, -11, -8, -8, -11, -17, -28 },
    { 3, 53, 45, 4, 4, 45, 53, 3, 33, 69, 51, 24, 24, 51, 69, 33, 9, 61, 36, 15, 15, 36, 61, 9, 1, 40, 22, 2, 2, 22, 40, 1, -8, 40, 19, 2, 2, 19, 40, -8, -25, 26, 0, -22, -22, 0, 26, -25, -65, -12, -42, -51, -51, -42, -12, -65, -98, -59, -74, -94, -94, -74, -59, -98 }
};

constexpr int eval_limit = 31800;
constexpr int R = 4;

long long nodes = 0;


static int evaluate(const Board& board)
{
    int evaluation = 0;

    for (const PieceType& i : piece_types)
    {
        Bitboard white_pieces = board.pieces(i, white);
        Bitboard black_pieces = board.pieces(i, black);

        while (white_pieces) evaluation += piece_value[i] + piece_table[i][white_pieces.pop()];
        while (black_pieces) evaluation -= piece_value[i] + piece_table[i][black_pieces.pop() ^ 56];
    }

    return evaluation;
}


static int quiesce(int alpha, const int beta, Board& board)
{
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
    ++ nodes;

    // Quiesce if end of alpha beta
    if (depth_left == 0) return quiesce(alpha, beta, board);

    // eval_limit evaluation if checkmate occurs at 50-move rule or 0 evaluation if 50-move rule
    if (board.isHalfMoveDraw()) return board.getHalfMoveDrawType().first == checkmate ? -eval_limit : 0;

    // 0 evaluation if threefold repetition or insufficient material
    if (board.isRepetition(1) || board.isInsufficientMaterial()) return 0;

    // Null move pruning
    if (!board.inCheck() && depth_left >= R)
    {
        board.makeNullMove();
        const int score = -alpha_beta(-beta, -beta + 1, depth_left - R, board, pv);
        board.unmakeNullMove();

        if (score >= beta) return score;
    }

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
                ++depth;

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
            std::cout << "id name BlueWhale-v1-7\n"
                << "id author StellarKitten\n"
                << "uciok\n";
        }

        else if (command == "ucinewgame") board = Board();

        else if (command == "isready") std::cout << "readyok\n";
    }
}
