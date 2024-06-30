#include "G3D/HaltonSequence.h"

namespace G3D {
    
    static void advanceSequence(int base, Point2int32& current) {
        ++current.x;
        if (current.x == current.y) {
            debugAssertM(current.x < INT_MAX / base, "Halton Sequence wrapping around");
            current.y *= base;
            current.x = 1;
        } else if (current.x % base == 0) {
            ++current.x;
        }
    }

    void HaltonSequence::next(Point2& p) {
        p.x = ((float)m_xCurrent.x) / m_xCurrent.y;
        p.y = ((float)m_yCurrent.x) / m_yCurrent.y;
        advanceSequence(m_base.x, m_xCurrent);
        advanceSequence(m_base.y, m_yCurrent);
    }
}