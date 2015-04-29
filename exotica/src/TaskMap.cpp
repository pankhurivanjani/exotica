#include "exotica/TaskMap.h"

namespace exotica
{
    TaskMap::TaskMap() : updateJacobian_(true)
    {

    }

    Scene_ptr TaskMap::getScene()
    {
        return scene_;
    }

    EReturn TaskMap::initBase(tinyxml2::XMLHandle & handle, Server_ptr & server,
            const Scene_map & scene_ptr)
    {
        //!< Clear flags and kinematic scene pointer
        Object::initBase(handle, server);
        if (!server)
        {
            INDICATE_FAILURE
            return FAILURE;
        }
        server_ = server;
        scene_ = Scene_ptr();  //!< Null pointer

        if (handle.FirstChildElement("Scene").ToElement())
        {
            LOCK(scene_lock_);  //!< Local lock
            const char * name = handle.FirstChildElement("Scene").ToElement()->Attribute("name");
            if (name == nullptr)
            {
                INDICATE_FAILURE
                return PAR_ERR;
            }
            auto it = scene_ptr.find(name);
            if (it == scene_ptr.end())
            {
                INDICATE_FAILURE
                return PAR_ERR;
            }
            scene_ = it->second;
        }
        else
        {
            ERROR("No scene was specified!");
            return PAR_ERR;
        }

        std::vector<std::string> tmp_eff(0);
        std::vector<KDL::Frame> tmp_offset(0);

        tinyxml2::XMLHandle segment_handle(handle.FirstChildElement("EndEffector").FirstChildElement("limb"));
        while (segment_handle.ToElement())
        {
            if (!segment_handle.ToElement()->Attribute("segment"))
            {
                INDICATE_FAILURE
                return FAILURE;
            }
            tmp_eff.push_back(segment_handle.ToElement()->Attribute("segment"));
            KDL::Frame temp_frame = KDL::Frame::Identity(); //!< Initialise to identity
            if (segment_handle.FirstChildElement("vector").ToElement())
            {
                Eigen::VectorXd temp_vector;
                if (!ok(getVector(*(segment_handle.FirstChildElement("vector").ToElement()), temp_vector)))
                {
                    INDICATE_FAILURE
                    return FAILURE;
                }
                if (temp_vector.size() != 3)
                {
                    return FAILURE;
                }
                temp_frame.p.x(temp_vector(0));
                temp_frame.p.y(temp_vector(1));
                temp_frame.p.z(temp_vector(2));
            }
            if (segment_handle.FirstChildElement("quaternion").ToElement())
            {
                Eigen::VectorXd temp_vector;
                if (!ok(getVector(*(segment_handle.FirstChildElement("quaternion").ToElement()), temp_vector)))
                {
                    INDICATE_FAILURE
                    return FAILURE;
                }
                if (temp_vector.size() != 4)
                {
                    INDICATE_FAILURE
                    return FAILURE;
                }
                temp_frame.M =
                        KDL::Rotation::Quaternion(temp_vector(1), temp_vector(2), temp_vector(3), temp_vector(0));
            }
            tmp_offset.push_back(temp_frame);
            segment_handle = segment_handle.NextSiblingElement("limb");
        }

        scene_->appendTaskMap(getObjectName(), tmp_eff, tmp_offset);
        if(ok(initDerived(handle)))
        {
            return SUCCESS;
        }
        else
        {
            ERROR("Failed to initialise task '"<<getObjectName() <<"'");
            return FAILURE;
        }
    }

    bool TaskMap::isRegistered(int t)
    {
        if(phiFlag_(t)==1)
        {
            if(phiCnt_!=phiFlag_.size())
            {
                WARNING("Task map '"<<object_name_<<"' is hasn't got phi registered at all time steps!");
            }
            if(updateJacobian_)
            {
                if(jacFlag_(t)==1)
                {
                    if(jacCnt_!=jacFlag_.size())
                    {
                        WARNING("Task map '"<<object_name_<<"' is hasn't got phi registered at all time steps!");
                    }
                    return true;
                }
                else
                {
                    INDICATE_FAILURE;
                    return false;
                }
            }
            else
            {
                return true;
            }
        }
        else
        {
            INDICATE_FAILURE;
            return false;
        }
    }

    EReturn TaskMap::setTimeSteps(const int T)
    {
        phi_.assign(T,Eigen::VectorXdRef_ptr());
        jac_.assign(T,Eigen::MatrixXdRef_ptr());
        phiFlag_.resize(T);
        phiFlag_.setZero();
        jacFlag_.resize(T);
        jacFlag_.setZero();
        phiCnt_=0;
        jacCnt_=0;
        return SUCCESS;
    }

    EReturn TaskMap::registerPhi(Eigen::VectorXdRef_ptr y, int t)
    {
        LOCK(map_lock_);
        phi_.at(t)=y;
        if(phiFlag_(t)==0)
        {
            phiFlag_(t)=1;
            phiCnt_++;
        }
        return SUCCESS;
    }

    EReturn TaskMap::registerJacobian(Eigen::MatrixXdRef_ptr J, int t)
    {
        LOCK(map_lock_);
        jac_.at(t)=J;
        if(jacFlag_(t)==0)
        {
            jacFlag_(t)=1;
            jacCnt_++;
        }
        return SUCCESS;
    }    

    bool TaskMap::getEffReferences()
    {
        if(ok(scene_->getForwardMap(object_name_, eff_phi_)))
        {
            if(updateJacobian_)
            {
                if(ok(scene_->getJacobian(object_name_, eff_jac_)))
                {
                    return true;
                }
                else
                {
                    INDICATE_FAILURE;
                    return false;
                }
            }
            else
            {
                return true;
            }
        }
        else
        {
            INDICATE_FAILURE;
            return false;
        }
    }
}
