#include <sstream>
#include <chrono>
#include "chess.hpp"

using namespace chess;

// Piece value and PST order
static constexpr PieceType piece_types[6] = { PieceType::PAWN, PieceType::KNIGHT, PieceType::BISHOP, PieceType::ROOK, PieceType::QUEEN, PieceType::KING };

// Piece value data (table 6): https://arxiv.org/pdf/2009.04374
static constexpr int piece_values[6] = { 100, 305, 333, 563, 950, 0 };

// PST data (release 16): https://github.com/official-stockfish/Stockfish
// Pawn PSTs are asymmetric
static constexpr int pst_mg[6][64] =
{
    { 0, 0, 0, 0, 0, 0, 0, 0, 2, 4, 11, 18, 16, 21, 9, -3, -9, -15, 11, 15, 31, 23, 6, -20, -3, -20, 8, 19, 39, 17, 2, -5, 11, -4, -11, 2, 11, 0, -12, 5, 3, -11, -6, 22, -8, -5, -14, -11, -7, 6, -2, -11, 4, -14, 10, -9, 0, 0, 0, 0, 0, 0, 0, 0 },
    { -175, -92, -74, -73, -73, -74, -92, -175, -77, -41, -27, -15, -15, -27, -41, -77, -61, -17, 6, 12, 12, 6, -17, -61, -35, 8, 40, 49, 49, 40, 8, -35, -34, 13, 44, 51, 51, 44, 13, -34, -9, 22, 58, 53, 53, 58, 22, -9, -67, -27, 4, 37, 37, 4, -27, -67, -201, -83, -56, -26, -26, -56, -83, -201 },
    { -37, -4, -6, -16, -16, -6, -4, -37, -11, 6, 13, 3, 3, 13, 6, -11, -5, 15, -4, 12, 12, -4, 15, -5, -4, 8, 18, 27, 27, 18, 8, -4, -8, 20, 15, 22, 22, 15, 20, -8, -11, 4, 1, 8, 8, 1, 4, -11, -12, -10, 4, 0, 0, 4, -10, -12, -34, 1, -10, -16, -16, -10, 1, -34 },
    { -31, -20, -14, -5, -5, -14, -20, -31, -21, -13, -8, 6, 6, -8, -13, -21, -25, -11, -1, 3, 3, -1, -11, -25, -13, -5, -4, -6, -6, -4, -5, -13, -27, -15, -4, 3, 3, -4, -15, -27, -22, -2, 6, 12, 12, 6, -2, -22, -2, 12, 16, 18, 18, 16, 12, -2, -17, -19, -1, 9, 9, -1, -19, -17 },
    { 3, -5, -5, 4, 4, -5, -5, 3, -3, 5, 8, 12, 12, 8, 5, -3, -3, 6, 13, 7, 7, 13, 6, -3, 4, 5, 9, 8, 8, 9, 5, 4, 0, 14, 12, 5, 5, 12, 14, 0, -4, 10, 6, 8, 8, 6, 10, -4, -5, 6, 10, 8, 8, 10, 6, -5, -2, -2, 1, -2, -2, 1, -2, -2 },
    { 271, 327, 271, 198, 198, 271, 327, 271, 278, 303, 234, 179, 179, 234, 303, 278, 195, 258, 169, 120, 120, 169, 258, 195, 164, 190, 138, 98, 98, 138, 190, 164, 154, 179, 105, 70, 70, 105, 179, 154, 123, 145, 81, 31, 31, 81, 145, 123, 88, 120, 65, 33, 33, 65, 120, 88, 59, 89, 45, -1, -1, 45, 89, 59 }
};

static constexpr int pst_eg[6][64] =
{
    { 0, 0, 0, 0, 0, 0, 0, 0, -8, -6, 9, 5, 16, 6, -6, -18, -9, -7, -10, 5, 2, 3, -8, -5, 7, 1, -8, -2, -14, -13, -11, -6, 12, 6, 2, -6, -5, -4, 14, 9, 27, 18, 19, 29, 30, 9, 8, 14, -1, -14, 13, 22, 24, 17, 7, 7, 0, 0, 0, 0, 0, 0, 0, 0 },
    { -96, -65, -49, -21, -21, -49, -65, -96, -67, -54, -18, 8, 8, -18, -54, -67, -40, -27, -8, 29, 29, -8, -27, -40, -35, -2, 13, 28, 28, 13, -2, -35, -45, -16, 9, 39, 39, 9, -16, -45, -51, -44, -16, 17, 17, -16, -44, -51, -69, -50, -51, 12, 12, -51, -50, -69, -100, -88, -56, -17, -17, -56, -88, -100 },
    { -40, -21, -26, -8, -8, -26, -21, -40, -26, -9, -12, 1, 1, -12, -9, -26, -11, -1, -1, 7, 7, -1, -1, -11, -14, -4, 0, 12, 12, 0, -4, -14, -12, -1, -10, 11, 11, -10, -1, -12, -21, 4, 3, 4, 4, 3, 4, -21, -22, -14, -1, 1, 1, -1, -14, -22, -32, -29, -26, -17, -17, -26, -29, -32 },
    { -9, -13, -10, -9, -9, -10, -13, -9, -12, -9, -1, -2, -2, -1, -9, -12, 6, -8, -2, -6, -6, -2, -8, 6, -6, 1, -9, 7, 7, -9, 1, -6, -5, 8, 7, -6, -6, 7, 8, -5, 6, 1, -7, 10, 10, -7, 1, 6, 4, 5, 20, -5, -5, 20, 5, 4, 18, 0, 19, 13, 13, 19, 0, 18 },
    { -69, -57, -47, -26, -26, -47, -57, -69, -54, -31, -22, -4, -4, -22, -31, -54, -39, -18, -9, 3, 3, -9, -18, -39, -23, -3, 13, 24, 24, 13, -3, -23, -29, -6, 9, 21, 21, 9, -6, -29, -38, -18, -11, 1, 1, -11, -18, -38, -50, -27, -24, -8, -8, -24, -27, -50, -74, -52, -43, -34, -34, -43, -52, -74 },
    { 1, 45, 85, 76, 76, 85, 45, 1, 53, 100, 133, 135, 135, 133, 100, 53, 88, 130, 169, 175, 175, 169, 130, 88, 103, 156, 172, 172, 172, 172, 156, 103, 96, 166, 199, 199, 199, 199, 166, 96, 92, 172, 184, 191, 191, 184, 172, 92, 47, 121, 116, 131, 131, 116, 121, 47, 11, 59, 73, 78, 78, 73, 59, 11 }
};

static constexpr int phase_limit = 30;
static constexpr int flip_const = 56;
static constexpr int eval_limit = 31800;
static constexpr int R = 4;

static int64_t nodes = 0;


static struct tt_entry
{
    uint64_t hash = 0;
    int depth = 0;
    int score = 0;
    Move move = Move::NULL_MOVE;
};


// Each TT entry is 8 + 4 + 4 + 2 = 20 B
// 1 GB / 20 B = 5E7
// In the future, implement TT size as a UCI command
static constexpr size_t tt_size = 5E7;
static std::vector<tt_entry> tt(tt_size);


static inline int evaluate(const Board& board)
{
    // King not included in phase calculation
    int phase = -2;
    int mg = 0;
    int eg = 0;

    // Loop through all piece types
    for (const PieceType& i : piece_types)
    {
        // Get pieces
        Bitboard wp = board.pieces(i, Color::WHITE);
        Bitboard bp = board.pieces(i, Color::BLACK);

        // Add number of pieces to phase
        phase += wp.count() + bp.count();

        // Loop through all pieces and add/subtract value and location
        while (wp)
        {
            const int sq = wp.pop();
            mg += piece_values[i] + pst_mg[i][sq];
            eg += piece_values[i] + pst_eg[i][sq];
        }

        while (bp)
        {
            const int sq = bp.pop() ^ flip_const;
            mg -= piece_values[i] + pst_mg[i][sq];
            eg -= piece_values[i] + pst_eg[i][sq];
        }
    }

    return (mg * phase + eg * (phase_limit - phase)) / phase_limit;
}


static inline int mvv_lva(const Board& board, const Move& move) { return piece_values[board.at(move.to()).type()] - piece_values[board.at(move.from()).type()]; }


static int quiesce(int alpha, const int beta, Board& board)
{
    ++nodes;

    // If black to move, get negative best_value
    int best_value = evaluate(board) * (board.sideToMove() == Color::WHITE ? 1 : -1);

    if (best_value >= beta) return best_value;

    // Delta pruning
    if (best_value < alpha - piece_values[4]) return alpha;

    if (best_value > alpha) alpha = best_value;

    // Get captures
    Movelist captures;
    movegen::legalmoves<movegen::MoveGenType::CAPTURE>(captures, board);

    // Order captures by MVV-LVA
    std::sort(captures.begin(), captures.end(), [&](const Move& i, const Move& j) { return mvv_lva(board, i) > mvv_lva(board, j); });

    // Loop through all captures
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


static inline int order_pst(const Board& board, const Move& move)
{
    // King not included in phase calculation
    int phase = -2;

    // Loop through all piece types
    for (const PieceType& i : piece_types)
    {
        // Get pieces
        Bitboard wp = board.pieces(i, Color::WHITE);
        Bitboard bp = board.pieces(i, Color::BLACK);

        // Add number of pieces to phase
        phase += wp.count() + bp.count();
    }

    // Get piece start and end location
    const Square sq_from = move.from();
    const Square sq_to = move.to();

    // Get piece and piece type
    const Piece piece = board.at(sq_from);
    const PieceType pt = piece.type();

    // Calculate flip
    const int flip = (piece.color() == Color::BLACK) * flip_const;

    // Get and flip index
    const int from = sq_from.index() ^ flip;
    const int to = sq_to.index() ^ flip;

    // Get values from PST
    const int mg = pst_mg[pt][to] - pst_mg[pt][from];
    const int eg = pst_eg[pt][to] - pst_eg[pt][from];

    return (mg * phase + eg * (phase_limit - phase)) / phase_limit;
}


static int negamax(int alpha, const int beta, const int depth_left, Board& board, std::vector<Move>& pv)
{
    ++nodes;

    // Quiesce if depth is 0
    if (depth_left == 0) return quiesce(alpha, beta, board);

    // eval_limit evaluation if checkmate occurs at 50-move rule or 0 evaluation if 50-move rule
    if (board.isHalfMoveDraw()) return board.getHalfMoveDrawType().first == GameResultReason::CHECKMATE ? -eval_limit : 0;

    // 0 evaluation if threefold repetition or insufficient material
    if (board.isRepetition(1) || board.isInsufficientMaterial()) return 0;
    
    // Get Zobrist hash and probe TT
    const uint64_t hash = board.zobrist();
    tt_entry& entry = tt[hash & (tt_size - 1)];
    const bool hash_exist = (entry.hash == hash);

    // Evaluation from higher depth if hash in TT
    if (hash_exist && entry.depth >= depth_left) { return entry.score; }

    // Null move pruning
    if (!board.inCheck() && depth_left >= R)
    {
        board.makeNullMove();
        const int score = -negamax(-beta, -beta + 1, depth_left - R, board, pv);
        board.unmakeNullMove();

        if (score >= beta) return score;
    }

    // Get moves
    Movelist moves;
    movegen::legalmoves<movegen::MoveGenType::ALL>(moves, board);

    // eval_limit evaluation if checkmate or 0 evaluation if stalemate
    if (moves.empty()) return board.inCheck() ? -eval_limit : 0;

    const bool pv_empty = pv.empty();
    int loc = 0;

    // Order PV move first
    if (!pv_empty)
    {
        const Movelist::iterator it = std::find(moves.begin(), moves.end(), pv[0]);
        if (it != moves.end()) std::swap(*it, moves[loc]), loc++;
    }

    // Order TT move second
    if (hash_exist)
    {
        const Movelist::iterator it = std::find(moves.begin() + loc, moves.end(), entry.move);
        if (it != moves.end()) std::swap(*it, moves[loc]), loc++;
    }

    // Get iterator to separate captures and quiet moves
    const Movelist::iterator it = std::stable_partition(moves.begin() + loc, moves.end(), [&](const Move& i) { return board.isCapture(i); });

    // Order captures first by MVV-LVA
    std::sort(moves.begin() + loc, it, [&](const Move& i, const Move& j) { return mvv_lva(board, i) > mvv_lva(board, j); });

    // Order quiet moves by PST
    std::sort(it, moves.end(), [&](const Move& i, const Move& j) { return order_pst(board, i) > order_pst(board, j); });

    int move_count = 0;
    std::vector<Move> child_pv;
    int best_value = -eval_limit;

    // Loop through all moves
    for (const Move& i : moves)
    {
        ++move_count;
        child_pv.clear();
        int score = 0;

        board.makeMove(i);

        // Late move reduction
        if (depth_left >= 2)
        {
            const int reduction = static_cast<int>(std::round(log(depth_left) * log(move_count) / 2));
            score = -negamax(-beta, -alpha, depth_left - 1 - reduction, board, child_pv);

            if (score > alpha) { score = -negamax(-beta, -alpha, depth_left - 1, board, child_pv); }
        }
        
        else { score = -negamax(-beta, -alpha, depth_left - 1, board, child_pv); }
        
        board.unmakeMove(i);

        if (score >= beta) return score;
        if (score > best_value)
        {
            best_value = score;
            if (score > alpha)
            {
                alpha = score;
                pv = child_pv;
                pv.insert(pv.begin(), i);
            }
        }
    }

    // Append entry to TT if no entry or at higher depth
    if (!hash_exist || depth_left >= entry.depth)
    {
        entry.hash = hash;
        entry.score = best_value;
        entry.depth = depth_left;
        entry.move = pv_empty ? Move::NULL_MOVE : pv[0];
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

                const int score = negamax(-eval_limit, eval_limit, depth, board, pv);
                const int64_t time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time).count();

                // Multiply by 1000 to convert millisecond to second
                const int64_t nps = (time > 0 ? nodes * 1000 / time : 0);

                std::cout << "info depth " << depth
                          << " score cp " << score
                          << " time " << time
                          << " nodes " << nodes
                          << " nps " << nps
                          << " pv";
                for (const Move& i : pv) std::cout << " " << uci::moveToUci(i);
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
            std::cout << "id name BlueWhale-v1-9\n"
                      << "id author StellarKitten\n"
                      << "uciok\n";
        }

        else if (command == "ucinewgame") board = Board();

        else if (command == "isready") std::cout << "readyok\n";
    }
}
