

#pragma once

#include <QMetaType>

class QTextStream;

namespace caf
{
//==================================================================================================
//==================================================================================================
class Tristate
{
public:
    enum class State
    {
        False,
        PartiallyTrue,
        True
    };

public:
    Tristate();

    void operator=( const State& state );

    bool operator==( const Tristate& other ) const;
    bool operator==( State state ) const;
    bool operator!=( const Tristate& other ) const;

    State state() const;

    bool isTrue() const;
    bool isPartiallyTrue() const;
    bool isFalse() const;

    QString text() const;
    void    setFromText( const QString& valueText );

protected:
    State m_state;
};

} // end namespace caf

//==================================================================================================
// Overload of QTextStream for caf::Triplet
//==================================================================================================
QTextStream& operator>>( QTextStream& str, caf::Tristate& triplet );
QTextStream& operator<<( QTextStream& str, const caf::Tristate& triplet );

Q_DECLARE_METATYPE( caf::Tristate );
