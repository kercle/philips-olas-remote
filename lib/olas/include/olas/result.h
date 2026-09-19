#pragma once

#include <optional>

template <typename T_ERR>
class Result {
    std::optional<T_ERR> err_payload;

    RadioTransmitterResult(std::optional<T_ERR> err_payload)
        : err_payload(err_payload)
    {
    }

public:
    inline static RadioTransmitterResult ok()
    {
        return RadioTransmitterResult(std::nullopt);
    }

    inline static RadioTransmitterResult err(T_ERR code)
    {
        return RadioTransmitterResult(std::optional<T_ERR>(code));
    }

    bool is_err() const;
    bool is_ok() const;

    int16_t error_payload() const;
};
