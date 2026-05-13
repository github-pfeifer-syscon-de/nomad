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

#include <iostream>
#include <memory>
#include <cstring>
#include <StringUtils.hpp>
#include <psc_format.hpp>
#include <vector>
#include <genericimg/psc_format.hpp>
#include <dlfcn.h>
#include <functional>

#include "SaneScanDevice.hpp"

static void* lib_handle{};
// contain our template magic into this struct
template <typename T>
struct sane_dynaptr
{
    using type_ptr = std::add_pointer<T>::type;
    std::function<T> sane_fun;
    void bind(void* lib_handle, const char* name)
    {
        sane_fun = reinterpret_cast<type_ptr>(dlsym(lib_handle, name));
    }
    operator bool()
    {
        return sane_fun.operator bool();
    }
    template <typename ...Params>
    auto operator ()(Params&&... params)
    {
        return sane_fun(std::forward<Params>(params)...);
    }
    template <typename R>
    R target()
    {
        return sane_fun.template target<R>();
    }
};

// stealing the definition should reduce the undetected incompatibilities
static sane_dynaptr<decltype(sane_init)> dyn_sane_init;
static sane_dynaptr<decltype(sane_exit)> dyn_sane_exit;
static sane_dynaptr<decltype(sane_get_devices)> dyn_sane_get_devices;
static sane_dynaptr<decltype(sane_open)> dyn_sane_open;
static sane_dynaptr<decltype(sane_close)> dyn_sane_close;
static sane_dynaptr<decltype(sane_get_option_descriptor)> dyn_sane_get_option_descriptor;
static sane_dynaptr<decltype(sane_control_option)> dyn_sane_control_option;
static sane_dynaptr<decltype(sane_get_parameters)> dyn_sane_get_parameters;
static sane_dynaptr<decltype(sane_start)> dyn_sane_start;
static sane_dynaptr<decltype(sane_read)> dyn_sane_read;
static sane_dynaptr<decltype(sane_cancel)> dyn_sane_cancel;
static sane_dynaptr<decltype(sane_set_io_mode)> dyn_sane_set_io_mode;
static sane_dynaptr<decltype(sane_get_select_fd)> dyn_sane_get_select_fd;
static sane_dynaptr<decltype(sane_strstatus)> dyn_sane_strstatus;
static bool sane_initialized{};

SaneException::SaneException(std::string_view where, SANE_Status sane_status)
: std::runtime_error(psc::fmt::format("Sane exception {} status {}"
                                , where
                                , dyn_sane_strstatus.operator bool()
                                    ? std::string(dyn_sane_strstatus(sane_status))
                                    : std::to_string(sane_status)))
{
}

SaneConstraint::SaneConstraint()
{
}

SaneConstraintRange::SaneConstraintRange(double min, double max, double quant)
: m_min{min}
, m_max{max}
, m_quant{quant}
{
}

double
SaneConstraintRange::getMin()
{
    return m_min;
}

double
SaneConstraintRange::getMax()
{
    return m_max;
}

double
SaneConstraintRange::getQuant()
{
    return m_quant;
}

SaneConstraintDiscreteInt::SaneConstraintDiscreteInt(SANE_Int size)
{
    m_values.reserve(size);
}

void
SaneConstraintDiscreteInt::add(SANE_Int value)
{
    m_values.push_back(value);
    std::sort(m_values.begin(), m_values.end());    // show ascending
}

std::vector<SANE_Int>
SaneConstraintDiscreteInt::getValues()
{
    return m_values;
}

SaneConstraintDiscreteString::SaneConstraintDiscreteString()
{
}

void
SaneConstraintDiscreteString::add(const std::string& value)
{
    m_values.push_back(value);
}

std::vector<std::string>
SaneConstraintDiscreteString::getValues()
{
    return m_values;
}

SaneScanOption::SaneScanOption(SANE_Handle sane_handle, const SANE_Option_Descriptor* sane_opt, SANE_Int nopt)
: m_sane_handle{sane_handle}
, m_sane_opt{sane_opt}
, m_nopt{nopt}
{
    m_name = sane_opt->name;
}

std::string
SaneScanOption::getName()
{
    return m_name;
}

SANE_Int
SaneScanOption::getIntValue()
{
    return 0;
}

std::string
SaneScanOption::getStrValue()
{
    return "";
}

bool
SaneScanOption::getBoolValue()
{
    return false;
}

double
SaneScanOption::getFixValue()
{
    return 0.0;
}

std::string
SaneScanOption::getInfo()
{
    std::string info;
    info.reserve(256);
    info = psc::fmt::format("sane opt {} cap {:#x} {} {} type {} unit {} size {} "
        ,m_nopt, m_sane_opt->cap
        ,(SANE_OPTION_IS_ACTIVE(m_sane_opt->cap) ? "active" : "inactive")
        ,(SANE_OPTION_IS_SETTABLE(m_sane_opt->cap) ? "setable" : "")
        ,SaneScanOption::decodeType(m_sane_opt->type)
        ,SaneScanOption::getUnitString(m_sane_opt->unit)
        ,m_sane_opt->size);

    if (m_sane_opt->size > 0) {
        info += "\n    value:" + getStrValue();
        //std::vector<char> value(m_sane_opt->size);
        //SANE_Status sane_opt_status = sane_control_option(m_sane_handle, m_nopt, SANE_ACTION_GET_VALUE, value.data(), 0);
        //if (sane_opt_status == SANE_STATUS_GOOD) {
        //    int size = std::min(m_sane_opt->size, 64);
        //    info += "\n     " + StringUtils::hexdump(value.data(), size);
        //    if (size < m_sane_opt->size) {
        //        info += "\n     ...";
        //    }
        //}
        //else {
        //    std::cout << "Error query value " << sane_opt_status << std::endl;
        //}
    }
    auto constr = getConstraints();
    auto rangeConstr = std::dynamic_pointer_cast<SaneConstraintRange>(constr);
    if (rangeConstr) {
        info += psc::fmt::format("\n    constr: [{}, {}] quant {}"
                    , rangeConstr->getMin()
                    , rangeConstr->getMax()
                    , rangeConstr->getQuant());
    }
    auto intConstr = std::dynamic_pointer_cast<SaneConstraintDiscreteInt>(constr);
    if (intConstr) {
        info += "\n    constr: ";
        auto start = info.size();
        for (auto n : intConstr->getValues()) {
            if (info.size() > start) {
                info += ",";
            }
            info += psc::fmt::format("{}", n);
        }
    }
    auto strConstr = std::dynamic_pointer_cast<SaneConstraintDiscreteString>(constr);
    if (strConstr) {
        info += "\n    constr: ";
        auto start = info.size();
        for (auto s : strConstr->getValues()) {
            if (info.size() > start) {
                info += ",";
            }
            info += s;
        }
    }
    return info;
}

std::shared_ptr<SaneConstraint>
SaneScanOption::getConstraints()
{
    return nullptr;
}

std::string
SaneScanOption::decodeType(SANE_Value_Type type)
{
    switch (type) {
        case SANE_TYPE_INT:
            return "int";
        case SANE_TYPE_BOOL:
            return "bool";
        case SANE_TYPE_FIXED:
            return "fixed";
        case SANE_TYPE_GROUP:
            return "group";
        case SANE_TYPE_BUTTON:
            return "button";
        case SANE_TYPE_STRING:
            return "string";
        default:
            return psc::fmt::format("type ({})", static_cast<int>(type));
    }
}

std::string
SaneScanOption::getUnitString(SANE_Unit unit)
{
    switch (unit) {
        case SANE_UNIT_NONE:
            return "none";
        case SANE_UNIT_PIXEL:
            return "px";
        case SANE_UNIT_BIT:
            return "bit";
        case SANE_UNIT_MM:
            return "mm";
        case SANE_UNIT_DPI:
            return "dpi";
        case SANE_UNIT_PERCENT:
            return "%";
        case SANE_UNIT_MICROSECOND:
            return "µs";
        default:
            return psc::fmt::format("unit ({})", static_cast<int>(unit));
    }
}

SaneStringOption::SaneStringOption(SANE_Handle sane_handle, const SANE_Option_Descriptor* sane_opt, SANE_Int nopt)
: SaneScanOption(sane_handle, sane_opt, nopt)
{
}

std::string
SaneStringOption::getStrValue()
{
    std::string svalue;
    std::vector<char> value(m_sane_opt->size);
    SANE_Status sane_opt_status = dyn_sane_control_option(m_sane_handle, m_nopt, SANE_ACTION_GET_VALUE, value.data(), nullptr);
    if (sane_opt_status == SANE_STATUS_GOOD) {
        const char* cval = value.data();
        //svalue = std::string(cval, std::strlen(cval));  // m_sane_opt->size may include more \0
        svalue = cval;
        //m_value += " constr " + getConstraints();
        //StringUtils::hexdump(value.data(), value.size());
    }
    else {
        throw SaneException("sane_control_option string ", sane_opt_status);
    }
    return svalue;
}

void
SaneStringOption::setStrValue(const std::string& sane_value)
{
    SANE_Int info;
    std::cout << "SaneStringOption::setStrValue " << m_name << "=" << sane_value << std::endl;
    auto void_val = reinterpret_cast<void*>(const_cast<char*>(sane_value.c_str())); // pass param as the api requires
    SANE_Status sane_opt_status = dyn_sane_control_option(m_sane_handle, m_nopt, SANE_ACTION_SET_VALUE, void_val, &info);
    if (sane_opt_status == SANE_STATUS_GOOD) {
        std::cout << "SaneStringOption::setStrValue info " << info << std::endl;
        //StringUtils::hexdump(value.data(), value.size());
    }
    else {
        throw SaneException("sane_control_option set str ", sane_opt_status);
    }
}

std::shared_ptr<SaneConstraint>
SaneStringOption::getConstraints()
{
    std::shared_ptr<SaneConstraint> constr;
    if (m_sane_opt->constraint_type == SANE_CONSTRAINT_STRING_LIST) {
        auto constrStr = std::make_shared<SaneConstraintDiscreteString>();
        for (size_t opt = 0; m_sane_opt->constraint.string_list[opt]; ++opt) {
            constrStr->add(std::string(m_sane_opt->constraint.string_list[opt]));
        }
        constr = constrStr;
    }
    return constr;
}


SaneBoolOption::SaneBoolOption(SANE_Handle sane_handle, const SANE_Option_Descriptor* sane_opt, SANE_Int nopt)
: SaneScanOption(sane_handle, sane_opt, nopt)
{
}

bool
SaneBoolOption::getBoolValue()
{
    SANE_Bool value{};
    SANE_Status sane_opt_status = dyn_sane_control_option(m_sane_handle, m_nopt, SANE_ACTION_GET_VALUE, &value, nullptr);
    if (sane_opt_status == SANE_STATUS_GOOD) {
        //StringUtils::hexdump(value.data(), value.size());
    }
    else {
        throw SaneException("sane_control_option get bool ", sane_opt_status);
    }
    return value;
}

void
SaneBoolOption::setBoolValue(SANE_Bool sane_value)
{
    SANE_Int info;
    std::cout << "SaneStringOption::setBoolValue " << m_name << "=" << std::boolalpha << sane_value << std::endl;
    SANE_Status sane_opt_status = dyn_sane_control_option(m_sane_handle, m_nopt, SANE_ACTION_SET_VALUE, &sane_value, &info);
    if (sane_opt_status == SANE_STATUS_GOOD) {
        std::cout << "SaneBoolOption::setBoolValue info " << info << std::endl;
        //StringUtils::hexdump(value.data(), value.size());
    }
    else {
        throw SaneException("sane_control_option set bool ", sane_opt_status);
    }
}

std::string
SaneBoolOption::getStrValue()
{
    return psc::fmt::format("{}", getBoolValue());
}

SaneIntOption::SaneIntOption(SANE_Handle sane_handle, const SANE_Option_Descriptor* sane_opt, SANE_Int nopt)
: SaneScanOption(sane_handle, sane_opt, nopt)
{
}

SANE_Int
SaneIntOption::getIntValue()
{
    SANE_Int value{};
    SANE_Status sane_opt_status = dyn_sane_control_option(m_sane_handle, m_nopt, SANE_ACTION_GET_VALUE, &value, nullptr);
    if (sane_opt_status != SANE_STATUS_GOOD) {
        throw SaneException("sane_control_option int ", sane_opt_status);
    }
    return value;
}

std::string
SaneIntOption::getStrValue()
{
    return psc::fmt::format("{}", getIntValue());
}

void
SaneIntOption::setIntValue(SANE_Int sane_value)
{
    SANE_Int info;
    std::cout << "SaneStringOption::setIntValue " << m_name << "=" <<  sane_value << std::endl;
    SANE_Status sane_opt_status = dyn_sane_control_option(m_sane_handle, m_nopt, SANE_ACTION_SET_VALUE, &sane_value, &info);
    if (sane_opt_status == SANE_STATUS_GOOD) {
        std::cout << "SaneIntOption::setIntValue info " << info << std::endl;
        //StringUtils::hexdump(value.data(), value.size());
    }
    else {
        throw SaneException("sane_control_option set int ", sane_opt_status);
    }
}


std::shared_ptr<SaneConstraint>
SaneIntOption::getConstraints()
{
    std::shared_ptr<SaneConstraint> constr;
    switch (m_sane_opt->constraint_type) {
        case SANE_CONSTRAINT_RANGE:
            constr = std::make_shared<SaneConstraintRange>(
                m_sane_opt->constraint.range->min
                ,m_sane_opt->constraint.range->max
                ,m_sane_opt->constraint.range->quant);
            break;
        case SANE_CONSTRAINT_WORD_LIST:{
            auto constrInt = std::make_shared<SaneConstraintDiscreteInt>(m_sane_opt->constraint.word_list[0]);
            for (SANE_Int opt = 1; opt <= m_sane_opt->constraint.word_list[0]; ++opt) {
                constrInt->add(m_sane_opt->constraint.word_list[opt]);
            }
            constr = constrInt;
            break;
        }
        default:
            break;
    }
    return constr;
}


SaneFixedOption::SaneFixedOption(SANE_Handle sane_handle, const SANE_Option_Descriptor* sane_opt, SANE_Int nopt)
: SaneScanOption(sane_handle, sane_opt, nopt)
{
}

double
SaneFixedOption::getFixValue()
{
    double dval{};
    SANE_Fixed value{};
    SANE_Status sane_opt_status = dyn_sane_control_option(m_sane_handle, m_nopt, SANE_ACTION_GET_VALUE, &value, nullptr);
    if (sane_opt_status == SANE_STATUS_GOOD) {
        dval = SANE_UNFIX(value);
    }
    else {
        throw SaneException("SaneFixedOption::getValue", sane_opt_status);
    }
    return dval;
}

void
SaneFixedOption::setFixValue(double sane_value)
{
    SANE_Int info;
    std::cout << "SaneFixedOption::setFixValue " << m_name << "=" <<  sane_value << std::endl;
    SANE_Fixed fixed = SANE_FIX(sane_value);
    SANE_Status sane_opt_status = dyn_sane_control_option(m_sane_handle, m_nopt, SANE_ACTION_SET_VALUE, &fixed, &info);
    if (sane_opt_status == SANE_STATUS_GOOD) {
        std::cout << "SaneIntOption::setFixValue info " << info << std::endl;
        //StringUtils::hexdump(value.data(), value.size());
    }
    else {
        throw SaneException("sane_control_option set fixed ", sane_opt_status);
    }
}


std::string
SaneFixedOption::getStrValue()
{
    return psc::fmt::format("{}", getFixValue());
}

std::shared_ptr<SaneConstraint>
SaneFixedOption::getConstraints() {
    std::shared_ptr<SaneConstraint> constr;
    if (m_sane_opt->constraint_type == SANE_CONSTRAINT_RANGE) {
        constr = std::make_shared<SaneConstraintRange>(
            SANE_UNFIX(m_sane_opt->constraint.range->min)
            ,SANE_UNFIX(m_sane_opt->constraint.range->max)
            ,SANE_UNFIX(m_sane_opt->constraint.range->quant));
    }
    return constr;
}

SaneGroupOption::SaneGroupOption(SANE_Handle sane_handle, const SANE_Option_Descriptor* sane_opt, SANE_Int nopt)
: SaneScanOption(sane_handle, sane_opt, nopt)
{
    //std::cout << "SaneGroupOption::SaneGroupOption " << m_sane_opt->name << std::endl;
    //if (m_sane_opt->cap & SANE_CAP_ADVANCED) {
    //    std::cout << "   advanced" << std::endl;
        /* advanced group: put all options to the advanced options window */
        //standard_parent = xsane_back_gtk_group_new(advanced_vbox, title);
        //advanced_parent = standard_parent;
        //elem->widget = standard_parent;
    //}
    //else {
    //    std::cout << "   default" << std::endl;
        /* we create the group twice. if no option is defined in one of this groups */
        /* then the group is not shown */

        /* standard group: put standard options to the standard options window */
        //standard_parent = xsane_back_gtk_group_new(standard_vbox, title);
        //elem->widget = standard_parent;

        /* standard group: put advanced options to the advanced options window */
        //advanced_parent = xsane_back_gtk_group_new(advanced_vbox, title);
        //elem->widget2 = advanced_parent;
    //}
}

SaneButtonOption::SaneButtonOption(SANE_Handle sane_handle, const SANE_Option_Descriptor* sane_opt, SANE_Int nopt)
: SaneScanOption(sane_handle, sane_opt, nopt)
{
}

SANE_Status
SaneScanDevice::load_sanelib(const std::string& sane_path)
{
    if (!lib_handle) {
        std::string sane_lookup = sane_path;
        if (sane_lookup.empty()) {
            sane_lookup = "/usr/lib/sane";
        }
        if (sane_lookup.at(sane_lookup.length() - 1) != '/') {
            sane_lookup += "/";
        }
        sane_lookup += "libsane-dll.so.1";
        lib_handle = dlopen(sane_lookup.c_str(), RTLD_NOW|RTLD_GLOBAL);
        if (lib_handle == nullptr) {
            std::cout << "Failed loading lib " <<  sane_lookup << " errno " << errno << " msg " <<  std::strerror(errno)  << std::endl;
            return SANE_STATUS_UNSUPPORTED;
        }
        std::cout << "Loaded lib successfully " << reinterpret_cast<void *>(lib_handle) << std::endl;
        dyn_sane_init.bind(lib_handle, "sane_init");
        dyn_sane_exit.bind(lib_handle, "sane_exit");
        dyn_sane_get_devices.bind(lib_handle, "sane_get_devices");
        dyn_sane_open.bind(lib_handle, "sane_open");
        dyn_sane_close.bind(lib_handle, "sane_close");
        dyn_sane_get_option_descriptor.bind(lib_handle, "sane_get_option_descriptor");
        dyn_sane_control_option.bind(lib_handle, "sane_control_option");
        dyn_sane_get_parameters.bind(lib_handle, "sane_get_parameters");
        dyn_sane_start.bind(lib_handle, "sane_start");
        dyn_sane_read.bind(lib_handle, "sane_read");
        dyn_sane_cancel.bind(lib_handle, "sane_cancel");
        dyn_sane_set_io_mode.bind(lib_handle, "sane_set_io_mode");
        dyn_sane_get_select_fd.bind(lib_handle, "sane_get_select_fd");
        dyn_sane_strstatus.bind(lib_handle, "sane_strstatus");
        if (!dyn_sane_init
         || !dyn_sane_exit
         || !dyn_sane_get_devices
         || !dyn_sane_open
         || !dyn_sane_close
         || !dyn_sane_get_option_descriptor
         || !dyn_sane_control_option
         || !dyn_sane_get_parameters
         || !dyn_sane_start
         || !dyn_sane_read
         || !dyn_sane_set_io_mode
         || !dyn_sane_get_select_fd
         || !dyn_sane_strstatus) {
            std::cout << "sane_init "<< dyn_sane_init.target<void*>() << "\n"
                      << "sane_exit " << dyn_sane_exit.target<void*>() << "\n"
                      << "sane_get_devices " << dyn_sane_get_devices.target<void*>() << "\n"
                      << "sane_open " << dyn_sane_open.target<void*>() << "\n"
                      << "sane_close " << dyn_sane_close.target<void*>() << "\n"
                      << "sane_get_option_descriptor " << dyn_sane_get_option_descriptor.target<void*>() << "\n"
                      << "sane_control_option " << dyn_sane_control_option.target<void*>() << "\n"
                      << "sane_get_parameters " << dyn_sane_get_parameters.target<void*>() << "\n"
                      << "sane_start " << dyn_sane_start.target<void*>() << "\n"
                      << "sane_read " << dyn_sane_read.target<void*>() << "\n"
                      << "sane_set_io_mode " << dyn_sane_set_io_mode.target<void*>() << "\n"
                      << "sane_get_select_fd " << dyn_sane_get_select_fd.target<void*>() << "\n"
                      << "sane_strstatus " << dyn_sane_strstatus.target<void*>() << std::endl;
            return SANE_STATUS_UNSUPPORTED;
         }
    }
    if (!sane_initialized) {
        //std::cout << "Sane major " << SANE_CURRENT_MAJOR << std::endl;
        SANE_Int sane_version{};
        auto sane_status = dyn_sane_init(&sane_version, nullptr);
        std::cout << "Sane version " << std::hex << sane_version << std::dec
                  << " init status " << sane_status
                  << std::endl;
        if (sane_version < SANE_MIN_VERSION) {
            dyn_sane_exit();
            std::cout << "The version may not be supported expecting >= " << SANE_MIN_VERSION << " maybe adjust"
                      << std::endl;
            return SANE_STATUS_UNSUPPORTED;
        }
        sane_initialized = sane_status == SANE_STATUS_GOOD;
        return sane_status;
    }
    return SANE_STATUS_GOOD;    // presume repeated init
}

void
SaneScanDevice::unload_sanelib()
{
    if (sane_initialized) {
        dyn_sane_exit();
        sane_initialized = false;
    }
    //if (flib_handle) {  // once linked keep references, or second invocation may fail
    //    dlclose(lib_handle);
    //    lib_handle = nullptr;
    //}
}

SaneScanDevice::SaneScanDevice(const SANE_Device* device)
{
    m_name = device->name;
    m_model = device->model;
    m_vendor = device->vendor;
    m_type = device->type;
}

SaneScanDevice::~SaneScanDevice()
{
    close();
}

std::string
SaneScanDevice::getId()
{
    return m_name;      // this is a technical name use on open e.g. genesys:libusb:001:013
}

std::string
SaneScanDevice::getName()
{
    return m_model;     // this is a human recognizeable name (could add vendor+type)
}

void
SaneScanDevice::close()
{
    if (m_sane_handle) {
        dyn_sane_close(m_sane_handle);
        m_sane_handle = nullptr;
    }
    m_options.clear();
}

template<typename T>
std::shared_ptr<T>
SaneScanDevice::getOption(std::string_view optName)
{
    auto options = getOptions();
    for (auto& option : options) {
        auto tOption = std::dynamic_pointer_cast<T>(option);
        if (tOption && tOption->getName() == optName) {
            return tOption;
        }
    }
    return nullptr;
}

std::shared_ptr<SaneIntOption>
SaneScanDevice::getIntOption(std::string_view optName)
{
    return getOption<SaneIntOption>(optName);
}

std::shared_ptr<SaneBoolOption>
SaneScanDevice::getBoolOption(std::string_view optName)
{
    return getOption<SaneBoolOption>(optName);
}

std::shared_ptr<SaneStringOption>
SaneScanDevice::getStrOption(std::string_view optName)
{
    return getOption<SaneStringOption>(optName);
}

std::shared_ptr<SaneFixedOption>
SaneScanDevice::getFixedOption(std::string_view optName)
{
    return getOption<SaneFixedOption>(optName);
}

void
SaneScanDevice::checkOpen()
{
    if (!m_sane_handle) {
        SANE_Status sane_status = dyn_sane_open(m_name.c_str(), &m_sane_handle);
        if (sane_status  != SANE_STATUS_GOOD) {
            throw SaneException("Device open", sane_status);
        }
    }
}

std::vector<std::shared_ptr<SaneScanOption>>
SaneScanDevice::getOptions()
{
    checkOpen();
    if (m_options.empty()) {
        const SANE_Option_Descriptor *sane_nopt = dyn_sane_get_option_descriptor(m_sane_handle, 0);
        SANE_Int optCnt{};
        if (sane_nopt->type == SANE_TYPE_INT) {
            SANE_Status sane_opt_status = dyn_sane_control_option(m_sane_handle, 0, SANE_ACTION_GET_VALUE, &optCnt, nullptr);
            if (sane_opt_status != SANE_STATUS_GOOD) {
                throw SaneException("sane_option query cnt", sane_opt_status);
            }
        }
        else {
            throw SaneException(psc::fmt::format("sane_option cnt not expected type {}", static_cast<int>(sane_nopt->type)), SANE_STATUS_INVAL);
        }
        for (SANE_Int nopt = 1; nopt < optCnt; ++nopt) {
            const SANE_Option_Descriptor* sane_opt = dyn_sane_get_option_descriptor(m_sane_handle, nopt);
            if (SANE_OPTION_IS_ACTIVE(sane_opt->cap)) {
                std::shared_ptr<SaneScanOption> option;
                switch (sane_opt->type) {
                    case SANE_TYPE_BOOL:
                        option = std::make_shared<SaneBoolOption>(m_sane_handle, sane_opt, nopt);
                        break;
                    case SANE_TYPE_INT:
                        option = std::make_shared<SaneIntOption>(m_sane_handle, sane_opt, nopt);
                        break;
                    case SANE_TYPE_FIXED:
                        option = std::make_shared<SaneFixedOption>(m_sane_handle, sane_opt, nopt);
                        break;
                    case SANE_TYPE_STRING:
                        option = std::make_shared<SaneStringOption>(m_sane_handle, sane_opt, nopt);
                        break;
                    case SANE_TYPE_GROUP:
                        option = std::make_shared<SaneGroupOption>(m_sane_handle, sane_opt, nopt);
                        break;
                    case SANE_TYPE_BUTTON:
                        option = std::make_shared<SaneButtonOption>(m_sane_handle, sane_opt, nopt);
                        break;
                    default:
                        std::cout << "Not handeld option type " <<  sane_opt->type << std::endl;
                        break;
                }
                if (option) {
                    m_options.push_back(option);
                }
            }
            else {
                std::cout << "SaneScanDevice::getOptions inactive " <<  sane_opt->name << std::endl;
            }
        }
    }
    return m_options;
}

SANE_Parameters
SaneScanDevice::getParameters()
{
    checkOpen();
    SANE_Parameters parameter;
    SANE_Status sane_status = dyn_sane_get_parameters(m_sane_handle, &parameter);
    if (sane_status != SANE_STATUS_GOOD) {
        throw SaneException("get parameters", sane_status);
    }
    return parameter;
}

void
SaneScanDevice::transfer(SaneScanParamNotify* scanPreview) {
    checkOpen();
    SANE_Status sane_status = dyn_sane_start(m_sane_handle);
    if (sane_status != SANE_STATUS_GOOD) {
        throw SaneException("sane_start", sane_status);
    }
    auto parameter = getParameters();
    std::cout << "param 2. format " << parameter.format
              << " last " <<  std::boolalpha << parameter.last_frame
              << " line_bytes " << parameter.bytes_per_line
              << " line_pixels " << parameter.pixels_per_line
              << " lines " << parameter.lines
              << " depth " << parameter.depth << std::endl;
    if (scanPreview) {
        scanPreview->setParameter(parameter);
    }

    std::vector<SANE_Byte> buf(parameter.bytes_per_line);   // the preview depends on transfer by line
    while (true) {
        SANE_Int len{};
        sane_status = dyn_sane_read(m_sane_handle, buf.data(), buf.size(), &len);
        if (sane_status == SANE_STATUS_EOF) {
            std::cout << "scan done " << std::endl;
            break;
        }
        if (sane_status != SANE_STATUS_GOOD) {
            throw SaneException("sane_read", sane_status);
        }
        if (len > 0) {
	    if (scanPreview) {
                scanPreview->append(buf, len);
	    }
        }
        else {
            break;
        }
    }
    close();
}

std::vector<std::shared_ptr<SaneScanDevice>>
SaneScanDevice::devices()
{
    std::vector<std::shared_ptr<SaneScanDevice>> devices;
    const SANE_Device** device_list;
    SANE_Status sane_status = dyn_sane_get_devices(&device_list, true);
    if (sane_status != SANE_STATUS_GOOD) {
        throw SaneException("get devices" ,sane_status);
    }
    devices.reserve(16);
    for (size_t ndev = 0; device_list[ndev]; ++ndev) {
        const SANE_Device* device = device_list[ndev];
        auto saneDevice = std::make_shared<SaneScanDevice>(device);
        devices.emplace_back(std::move(saneDevice));
    }
    return devices;
}
