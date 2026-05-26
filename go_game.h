#pragma once
#include <vector>
#include <string>
#include <set>
#include <utility>

enum class Player { None = 0, Black = 1, White = 2 };

struct Move {
    Player player;
    int x, z;       // 1–19, or 0,0 for pass
    bool isPass() const { return x == 0 && z == 0; }
};

class GoGame {
public:
    static const int SIZE = 19;

    GoGame();

    bool placeStone(int x, int z);   // returns false if illegal
    void pass();                      // current player passes
    bool isGameOver() const;          // true after two consecutive passes

    Player getCell(int x, int z) const;
    Player getCurrentPlayer() const;
    bool isValidMove(int x, int z) const;

    // For rendering — 0=empty, 1=black, 2=white
    int getBoardInt(int x, int z) const;

    void saveToSGF(const std::string& filename) const;
    bool loadFromSGF(const std::string& filename);

    const std::vector<Move>& getMoves() const { return moves; }

private:
    Player board[SIZE + 1][SIZE + 1];
    Player currentPlayer;
    int koX, koZ;                // ko-restricted point (0,0 if none)
    int consecutivePasses;
    std::vector<Move> moves;

    std::set<std::pair<int,int>> getGroup(int x, int z) const;
    bool groupHasLiberty(const std::set<std::pair<int,int>>& group) const;
    int  captureOpponentGroups(int x, int z);
    bool wouldBeSuicide(int x, int z, Player p) const;

    Player opponent(Player p) const;
    static char toSGFChar(int coord);
    static int  fromSGFChar(char c);
};