/*
 * NimBLEAttValue.h
 *
 *  Created: on March 18, 2021
 *      Author H2zero
 *
 */

#ifndef MAIN_NIMBLEATTVALUE_H_
#define MAIN_NIMBLEATTVALUE_H_
#include "nimconfig.h"
#if defined(CONFIG_BT_ENABLED)

#ifdef NIMBLE_ARDUINO_AVAILABLE
#include <Arduino.h>
#endif

#include "NimBLELog.h"

/****  FIX COMPILATION ****/
#undef min
#undef max
/**************************/

#include <string>
#include <vector>

#ifdef NIMBLE_ATT_TIMESTAMP_ENABLED
    #include <time.h>
#endif

#ifdef NIMBLE_ATT_TIMESTAMP_ENABLED
    #define NIMBLE_SET_TIMESTAMP(ts) m_timestamp = ts
#else
    #define NIMBLE_SET_TIMESTAMP(ts)
#endif

class NimBLEAttValue
{
    uint8_t*     m_attr_value = nullptr;
    uint16_t     m_attr_max_len = 0;
    uint16_t     m_attr_len = 0;
    uint16_t     m_capacity = 0;
#ifdef NIMBLE_ATT_TIMESTAMP_ENABLED
    time_t       m_timestamp = 0;
#endif
    void         deepCopy(const NimBLEAttValue & source);

public:
    NimBLEAttValue(uint16_t init_len = NIMBLE_ATT_INIT_LENGTH, uint16_t max_len = BLE_ATT_ATTR_MAX_LEN);
    NimBLEAttValue(const uint8_t *value, uint16_t len, uint16_t max_len = BLE_ATT_ATTR_MAX_LEN);
    NimBLEAttValue(const char *value, uint16_t max_len = BLE_ATT_ATTR_MAX_LEN)
                   :NimBLEAttValue((uint8_t*)value, strlen(value), max_len){}
    NimBLEAttValue(const std::string str, uint16_t max_len = BLE_ATT_ATTR_MAX_LEN)
                   :NimBLEAttValue((uint8_t*)str.data(), str.length(), max_len){}
    NimBLEAttValue(const std::vector<uint8_t> vec, uint16_t max_len = BLE_ATT_ATTR_MAX_LEN)
                   :NimBLEAttValue(&vec[0], vec.size(), max_len){}
    NimBLEAttValue(const NimBLEAttValue & source) { deepCopy(source); }
    NimBLEAttValue(NimBLEAttValue && source);
    ~NimBLEAttValue();

    uint16_t        getMaxLength() const { return m_attr_max_len; }
    uint16_t        getLength()    const { return m_attr_len; }
    uint8_t*        getValue()     const { return m_attr_value; }
    char*           c_str()        const { return (char*)m_attr_value; }
#ifdef NIMBLE_ATT_TIMESTAMP_ENABLED
    time_t          getTimeStamp() const { return m_timestamp; }
    void            setTimeStamp()       { m_timestamp = time(nullptr); }
#else
    time_t          getTimeStamp() const { return 0; }
    void            setTimeStamp()       { }
#endif


    bool            setValue(const uint8_t *value, uint16_t len);
    uint8_t*        getValue(time_t *timestamp);
    NimBLEAttValue& append(const uint8_t *value, uint16_t len);

    operator std::vector<uint8_t>() const { return std::vector<uint8_t>(m_attr_value, m_attr_value + m_attr_len); }
    operator std::string()    const { return std::string((char*)m_attr_value, m_attr_len); }
    operator const uint8_t*() const { return m_attr_value; }
    NimBLEAttValue& operator  +=(const NimBLEAttValue & source){return append(source.getValue(), source.getLength());}
    NimBLEAttValue& operator  =(const std::string & source){setValue((uint8_t*)source.data(), source.size()); return *this;}
    NimBLEAttValue& operator  =(NimBLEAttValue && source);
    NimBLEAttValue& operator  =(const NimBLEAttValue & source);

#ifdef NIMBLE_ARDUINO_AVAILABLE
    NimBLEAttValue(const String str):NimBLEAttValue((uint8_t*)str.c_str(), str.length()){}
    operator String() const { return String((char*)m_attr_value); }
#endif
};

inline NimBLEAttValue::NimBLEAttValue(const uint8_t *value, uint16_t len, uint16_t max_len) {
    m_attr_value   = (uint8_t*)calloc(len + 1, 1);
    assert(m_attr_value != nullptr && "No Mem");
    m_attr_max_len = max_len;
    m_attr_len     = len;
    m_capacity     = len;
    NIMBLE_SET_TIMESTAMP(0);
    memcpy(m_attr_value, value, len);
    m_attr_value[len] = '\0';
}

inline NimBLEAttValue::NimBLEAttValue(uint16_t init_len, uint16_t max_len) {
    m_attr_value   = (uint8_t*)calloc(init_len + 1, 1);
    assert(m_attr_value != nullptr && "No Mem");
    m_attr_max_len = max_len;
    m_attr_len     = 0;
    m_capacity     = init_len;
    NIMBLE_SET_TIMESTAMP(0);
}

inline NimBLEAttValue::NimBLEAttValue(NimBLEAttValue && source) {
    m_attr_value   = source.m_attr_value;
    m_attr_max_len = source.m_attr_max_len;
    m_attr_len     = source.m_attr_len;
    m_capacity     = source.m_capacity;
    NIMBLE_SET_TIMESTAMP(source.m_timestamp);
    source.m_attr_value = nullptr;
}

inline NimBLEAttValue::~NimBLEAttValue() {
    if(m_attr_value != nullptr) {
        free(m_attr_value);
    }
}

inline NimBLEAttValue& NimBLEAttValue::operator =(NimBLEAttValue && source) {
    if (this != &source){
        free(m_attr_value);

        m_attr_value   = source.m_attr_value;
        m_attr_max_len = source.m_attr_max_len;
        m_attr_len     = source.m_attr_len;
        m_capacity     = source.m_capacity;
        NIMBLE_SET_TIMESTAMP(source.m_timestamp);
        source.m_attr_value = nullptr;
    }
    return *this;
}

inline NimBLEAttValue& NimBLEAttValue::operator =(const NimBLEAttValue & source) {
    if (this != &source) {
        deepCopy(source);
    }
    return *this;
}


inline void NimBLEAttValue::deepCopy(const NimBLEAttValue & source) {
    uint8_t* res = (uint8_t*)realloc( m_attr_value, source.m_capacity + 1);

    assert(res && "deepCopy: realloc failed");

    ble_npl_hw_enter_critical();
    m_attr_value   = res;
    m_attr_max_len = source.m_attr_max_len;
    m_attr_len     = source.m_attr_len;
    m_capacity     = source.m_capacity;
    NIMBLE_SET_TIMESTAMP(source.m_timestamp);
    memcpy(m_attr_value, source.m_attr_value, m_attr_len + 1);
    ble_npl_hw_exit_critical(0);
}


inline uint8_t*  NimBLEAttValue::getValue(time_t *timestamp) {
    if(timestamp != nullptr) {
#ifdef NIMBLE_ATT_TIMESTAMP_ENABLED
        *timestamp = m_timestamp;
#else
        *timestamp = 0;
#endif
    }
    return m_attr_value;
}


inline bool NimBLEAttValue::setValue(const uint8_t *value, uint16_t len) {
    if (len > m_attr_max_len) {
        NIMBLE_LOGE("NimBLEAttValue", "value exceeds max, len=%u, max=%u", len, m_attr_max_len);
        return false;
    }

    uint8_t *res = m_attr_value;
    if (len > m_capacity) {
        res = (uint8_t*)realloc(m_attr_value, (len + 1));
    }
    assert(res && "setValue: realloc failed");

#ifdef NIMBLE_ATT_TIMESTAMP_ENABLED
    time_t t = time(nullptr);
#endif

    ble_npl_hw_enter_critical();
    m_attr_value = res;
    memcpy(m_attr_value, value, len);
    m_attr_value[len] = '\0';
    m_attr_len = len;
    NIMBLE_SET_TIMESTAMP(t);
    ble_npl_hw_exit_critical(0);
    return true;
}

inline NimBLEAttValue& NimBLEAttValue::append(const uint8_t *value, uint16_t len) {
    if ((m_attr_len + len) > m_attr_max_len) {
        NIMBLE_LOGE("NimBLEAttValue", "val > max, len=%u, max=%u", len, m_attr_max_len);
        return *this;
    }

    uint8_t* res = m_attr_value;
    if ((m_attr_len + len) > m_capacity) {
        res = (uint8_t*)realloc(m_attr_value, (m_attr_len + len + 1));
    }
    assert(res && "append: realloc failed");

#ifdef NIMBLE_ATT_TIMESTAMP_ENABLED
    time_t t = time(nullptr);
#endif

    ble_npl_hw_enter_critical();
    m_attr_value = res;
    memcpy(m_attr_value + m_attr_len, value, len);
    m_attr_len = m_attr_len + len;
    m_attr_value[m_attr_len] = '\0';
    NIMBLE_SET_TIMESTAMP(t);
    ble_npl_hw_exit_critical(0);

    return *this;
}

#endif /*(CONFIG_BT_ENABLED) */
#endif /* MAIN_NIMBLEATTVALUE_H_ */
