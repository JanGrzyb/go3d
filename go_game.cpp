#include "go_game.h"
#include <fstream>
#include <sstream>
#include <iostream>

GoGame::GoGame() {
    for (int i = 0; i <= SIZE; i++)
        for (int j = 0; j <= SIZE; j++)
            board[i][j] = Player::None;
    currentPlayer     = Player::Black;
    koX = koZ         = 0;
    consecutivePasses = 0;
}

Player GoGame::opponent(Player p) const {
    return (p == Player::Black) ? Player::White : Player::Black;
}

Player GoGame::getCell(int x, int z) const {
    return board[x][z];
}

Player GoGame::getCurrentPlayer() const {
    return currentPlayer;
}

int GoGame::getBoardInt(int x, int z) const {
    return static_cast<int>(board[x][z]);
}

bool GoGame::isGameOver() const {
    return consecutivePasses >= 2;
}

// ── Group & liberty helpers ──────────────────────────────────────────────────

std::set<std::pair<int,int>> GoGame::getGroup(int x, int z) const {
    std::set<std::pair<int,int>> group;
    std::vector<std::pair<int,int>> stack = {{x, z}};
    Player color = board[x][z];
    while (!stack.empty()) {
        auto [cx, cz] = stack.back(); stack.pop_back();
        if (!group.insert({cx, cz}).second) continue;
        int dx[] = {1,-1,0,0};
        int dz[] = {0,0,1,-1};
        for (int d = 0; d < 4; d++) {
            int nx = cx+dx[d], nz = cz+dz[d];
            if (nx>=1 && nx<=SIZE && nz>=1 && nz<=SIZE
                && board[nx][nz] == color
                && group.find({nx,nz}) == group.end())
                stack.push_back({nx, nz});
        }
    }
    return group;
}

bool GoGame::groupHasLiberty(const std::set<std::pair<int,int>>& group) const {
    int dx[] = {1,-1,0,0};
    int dz[] = {0,0,1,-1};
    for (auto [x, z] : group)
        for (int d = 0; d < 4; d++) {
            int nx = x+dx[d], nz = z+dz[d];
            if (nx>=1 && nx<=SIZE && nz>=1 && nz<=SIZE
                && board[nx][nz] == Player::None)
                return true;
        }
    return false;
}

int GoGame::captureOpponentGroups(int x, int z) {
    int captured = 0;
    int dx[] = {1,-1,0,0};
    int dz[] = {0,0,1,-1};
    Player opp = opponent(currentPlayer);
    for (int d = 0; d < 4; d++) {
        int nx = x+dx[d], nz = z+dz[d];
        if (nx<1||nx>SIZE||nz<1||nz>SIZE) continue;
        if (board[nx][nz] != opp) continue;
        auto group = getGroup(nx, nz);
        if (!groupHasLiberty(group)) {
            for (auto [gx, gz] : group) {
                board[gx][gz] = Player::None;
                captured++;
            }
        }
    }
    return captured;
}

bool GoGame::wouldBeSuicide(int x, int z, Player p) const {
    // Temporarily place the stone
    GoGame tmp = *this;
    tmp.board[x][z] = p;
    int dx[] = {1,-1,0,0};
    int dz[] = {0,0,1,-1};
    // First capture opponent groups (they might free liberties)
    Player opp = tmp.opponent(p);
    for (int d = 0; d < 4; d++) {
        int nx = x+dx[d], nz = z+dz[d];
        if (nx<1||nx>SIZE||nz<1||nz>SIZE) continue;
        if (tmp.board[nx][nz] != opp) continue;
        auto group = tmp.getGroup(nx, nz);
        if (!tmp.groupHasLiberty(group))
            for (auto [gx, gz] : group)
                tmp.board[gx][gz] = Player::None;
    }
    // Now check if our group has any liberty
    auto group = tmp.getGroup(x, z);
    return !tmp.groupHasLiberty(group);
}

// ── Move validation & placement ──────────────────────────────────────────────

bool GoGame::isValidMove(int x, int z) const {
    if (x < 1 || x > SIZE || z < 1 || z > SIZE) return false;
    if (board[x][z] != Player::None) return false;
    if (x == koX && z == koZ) return false;        // ko rule
    if (wouldBeSuicide(x, z, currentPlayer)) return false;
    return true;
}

bool GoGame::placeStone(int x, int z) {
    if (!isValidMove(x, z)) return false;

    board[x][z] = currentPlayer;
    int captured = captureOpponentGroups(x, z);

    // Update ko: ko only arises when exactly 1 stone captured
    // and the placed stone itself has exactly 1 liberty
    koX = koZ = 0;
    if (captured == 1) {
        // Find the single captured point — already removed, check neighbors
        int dx[] = {1,-1,0,0};
        int dz[] = {0,0,1,-1};
        for (int d = 0; d < 4; d++) {
            int nx = x+dx[d], nz = z+dz[d];
            if (nx>=1&&nx<=SIZE&&nz>=1&&nz<=SIZE
                && board[nx][nz] == Player::None) {
                // Check it was the captured stone (our group has only 1 liberty)
                auto group = getGroup(x, z);
                int libs = 0;
                for (auto [gx, gz] : group) {
                    for (int d2 = 0; d2 < 4; d2++) {
                        int nnx = gx+dx[d2], nnz = gz+dz[d2];
                        if (nnx>=1&&nnx<=SIZE&&nnz>=1&&nnz<=SIZE
                            && board[nnx][nnz] == Player::None)
                            libs++;
                    }
                }
                if (libs == 1) { koX = nx; koZ = nz; }
                break;
            }
        }
    }

    moves.push_back({currentPlayer, x, z});
    consecutivePasses = 0;
    currentPlayer = opponent(currentPlayer);
    return true;
}

void GoGame::pass() {
    moves.push_back({currentPlayer, 0, 0});
    consecutivePasses++;
    koX = koZ = 0;
    currentPlayer = opponent(currentPlayer);
}

// ── SGF ─────────────────────────────────────────────────────────────────────

char GoGame::toSGFChar(int coord) {
    return 'a' + (coord - 1); // 1→'a', 19→'s'
}

int GoGame::fromSGFChar(char c) {
    return (c - 'a') + 1;
}

void GoGame::saveToSGF(const std::string& filename) const {
    std::ofstream f(filename);
    if (!f) { std::cerr << "Cannot open " << filename << " for writing\n"; return; }

    f << "(;FF[4]GM[1]SZ[19]\n";
    for (const auto& m : moves) {
        if (m.isPass()) {
            f << ";" << (m.player == Player::Black ? "B" : "W") << "[]\n";
        } else {
            f << ";" << (m.player == Player::Black ? "B" : "W")
              << "[" << toSGFChar(m.x) << toSGFChar(m.z) << "]\n";
        }
    }
    f << ")\n";
    std::cout << "Game saved to " << filename << "\n";
}

bool GoGame::loadFromSGF(const std::string& filename) {
    std::ifstream f(filename);
    if (!f) { std::cerr << "Cannot open " << filename << "\n"; return false; }

    std::string content((std::istreambuf_iterator<char>(f)),
                         std::istreambuf_iterator<char>());

    // Reset state
    *this = GoGame();

    size_t i = 0;
    while (i < content.size()) {
        if (content[i] == ';') {
            i++;
            // skip whitespace
            while (i < content.size() && isspace(content[i])) i++;
            // read property key (B or W)
            std::string key;
            while (i < content.size() && isupper(content[i]))
                key += content[i++];
            // read value inside [...]
            if (i < content.size() && content[i] == '[') {
                i++; // skip '['
                std::string val;
                while (i < content.size() && content[i] != ']')
                    val += content[i++];
                if (i < content.size()) i++; // skip ']'

                if (key == "B" || key == "W") {
                    if (val.empty()) {
                        pass();
                    } else if (val.size() >= 2) {
                        int x = fromSGFChar(val[0]);
                        int z = fromSGFChar(val[1]);
                        placeStone(x, z);
                    }
                }
            }
        } else {
            i++;
        }
    }
    std::cout << "Loaded " << moves.size() << " moves from " << filename << "\n";
    return true;
}