/* -*- Mode: c++; c-basic-offset: 4; tab-width: 4; coding: utf-8; -*-  */
/*
 * Copyright (C) 2026 RPf
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#include <string_view>
#include <sane/sane.h>

class SaneException
: public std::runtime_error
{
public:
    SaneException(std::string_view where, SANE_Status sane_status);
    virtual ~SaneException() = default;

};

class SaneConstraint
{
public:
    SaneConstraint();
    explicit SaneConstraint(const SaneConstraint& other) = delete;
    virtual ~SaneConstraint() = default;

};

class SaneConstraintRange
: public SaneConstraint
{
public:
    SaneConstraintRange(double min, double max, double quant);
    explicit SaneConstraintRange(const SaneConstraintRange& other) = delete;
    virtual ~SaneConstraintRange() = default;

    double getMin();
    double getMax();
    double getQuant();
protected:
    double m_min;
    double m_max;
    double m_quant;
};

class SaneConstraintDiscreteInt
: public SaneConstraint
{
public:
    SaneConstraintDiscreteInt(SANE_Int size);
    explicit SaneConstraintDiscreteInt(const SaneConstraintDiscreteInt& other) = delete;
    virtual ~SaneConstraintDiscreteInt() = default;

    void add(SANE_Int value);
    std::vector<SANE_Int> getValues();
protected:
    std::vector<SANE_Int> m_values;
};

class SaneConstraintDiscreteString
: public SaneConstraint
{
public:
    SaneConstraintDiscreteString();
    explicit SaneConstraintDiscreteString(const SaneConstraintDiscreteString& other) = delete;
    virtual ~SaneConstraintDiscreteString() = default;
    void add(const std::string& value);
    std::vector<std::string> getValues();
protected:
    std::vector<std::string> m_values;
};

class SaneScanOption
{
public:
    SaneScanOption(SANE_Handle sane_handle, const SANE_Option_Descriptor* sane_opt, SANE_Int nopt);
    explicit SaneScanOption(const SaneScanOption& other) = delete;
    virtual ~SaneScanOption() = default;
    static std::string decodeType(SANE_Value_Type type);
    static std::string getUnitString(SANE_Unit unit);

    std::string getName();
    virtual SANE_Int getIntValue();
    virtual std::string getStrValue();
    virtual bool getBoolValue();
    virtual double getFixValue();
    virtual std::string getInfo();
    virtual std::shared_ptr<SaneConstraint> getConstraints();

protected:
    SANE_Handle m_sane_handle;
    const SANE_Option_Descriptor* m_sane_opt;
    SANE_Int m_nopt;
    std::string m_name;
private:
};

class SaneStringOption
: public SaneScanOption
{
public:
    SaneStringOption(SANE_Handle sane_handle, const SANE_Option_Descriptor* sane_opt, SANE_Int nopt);
    explicit SaneStringOption(const SaneStringOption& other) = delete;
    virtual ~SaneStringOption() = default;
    std::string getStrValue() override;
    void setStrValue(const std::string& sane_value);
    std::shared_ptr<SaneConstraint> getConstraints() override;

    static constexpr auto MODE_COLOR{"Color"};
    static constexpr auto MODE_GRAY{"Gray"};
private:
};

class SaneBoolOption
: public SaneScanOption
{
public:
    SaneBoolOption(SANE_Handle sane_handle, const SANE_Option_Descriptor* sane_opt, SANE_Int nopt);
    explicit SaneBoolOption(const SaneBoolOption& other) = delete;
    virtual ~SaneBoolOption() = default;
    bool getBoolValue() override;
    void setBoolValue(SANE_Bool sane_value);
    std::string getStrValue() override;

private:
};

class SaneIntOption
: public SaneScanOption
{
public:
    SaneIntOption(SANE_Handle sane_handle, const SANE_Option_Descriptor* sane_opt, SANE_Int nopt);
    explicit SaneIntOption(const SaneIntOption& other) = delete;
    virtual ~SaneIntOption() = default;
    SANE_Int getIntValue() override;
    void setIntValue(SANE_Int sane_value);
    std::string getStrValue() override;
    std::shared_ptr<SaneConstraint> getConstraints() override;

private:
};

class SaneFixedOption
: public SaneScanOption
{
public:
    SaneFixedOption(SANE_Handle sane_handle, const SANE_Option_Descriptor* sane_opt, SANE_Int nopt);
    explicit SaneFixedOption(const SaneFixedOption& other) = delete;
    virtual ~SaneFixedOption() = default;
    double getFixValue() override;
    void setFixValue(double value);
    std::string getStrValue() override;
    std::shared_ptr<SaneConstraint> getConstraints() override;

private:
};


class SaneGroupOption
: public SaneScanOption
{
public:
    SaneGroupOption(SANE_Handle sane_handle, const SANE_Option_Descriptor* sane_opt, SANE_Int nopt);
    explicit SaneGroupOption(const SaneGroupOption& other) = delete;
    virtual ~SaneGroupOption() = default;

private:
};

class SaneButtonOption
: public SaneScanOption
{
public:
    SaneButtonOption(SANE_Handle sane_handle, const SANE_Option_Descriptor* sane_opt, SANE_Int nopt);
    explicit SaneButtonOption(const SaneButtonOption& other) = delete;
    virtual ~SaneButtonOption() = default;
    //SANE_Int getIntValue() override;
    //std::shared_ptr<SaneConstraint> getConstraints() override;

private:
};

class SaneScanParamNotify
{
public:
    virtual ~SaneScanParamNotify() = default;
    virtual void setParameter(const SANE_Parameters& param) = 0;
    virtual void append(std::vector<SANE_Byte>& buf, SANE_Int len) = 0;
};

class SaneScanDevice
{
public:
    // required, use this call to link sane dynamically -> check return
    static SANE_Status load_sanelib(const std::string& sane_path);
    // calls after calling this function become unsane
    static void unload_sanelib();

    SaneScanDevice(const SANE_Device* device);
    explicit SaneScanDevice(const SaneScanDevice &other) = delete;
    virtual ~SaneScanDevice();
    std::string getId();
    std::string getName();
    void close();
    template<typename T> std::shared_ptr<T> getOption(std::string_view optName);
    std::shared_ptr<SaneIntOption> getIntOption(std::string_view optName);
    std::shared_ptr<SaneBoolOption> getBoolOption(std::string_view optName);
    std::shared_ptr<SaneStringOption> getStrOption(std::string_view optName);
    std::shared_ptr<SaneFixedOption> getFixedOption(std::string_view optName);
    std::vector<std::shared_ptr<SaneScanOption>> getOptions();     // may throw SaneException
    SANE_Parameters getParameters();
    static std::vector<std::shared_ptr<SaneScanDevice>> devices(); // may throw SaneException
    static constexpr auto MODE_OPTION{"mode"};
    static constexpr auto SOURCE_OPTION{"source"};
    static constexpr auto DEPTH_OPTION{"depth"};            // Int
    static constexpr auto TOPLEFTX_OPTION{"tl-x"};          // Fixed
    static constexpr auto TOPLEFTY_OPTION{"tl-y"};          // Fixed
    static constexpr auto BOTTOMRIGHTX_OPTION{"br-x"};      // Fixed
    static constexpr auto BOTTOMRIGHTY_OPTION{"br-y"};      // Fixed
    static constexpr auto BRIGHTNESS_OPTION{"brightness"};  // Int
    static constexpr auto CONTRAST_OPTION{"contrast"};      // Int
    static constexpr auto RESOLUTION_OPTION{"resolution"};  // Int (-> allow fixed as well)
    static constexpr auto PREVIEW_OPTION{"preview"};
    void transfer(SaneScanParamNotify* scanPreview);
    static constexpr auto SANE_MIN_VERSION{0x1000000};
protected:
    void checkOpen();
    static bool sane_initialized;


private:
    std::string m_name;
    std::string m_model;
    std::string m_vendor;
    std::string m_type;
    SANE_Handle m_sane_handle{};
    std::vector<std::shared_ptr<SaneScanOption>> m_options;


};

