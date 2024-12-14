#pragma once

#include <map>
#include <functional>
#include <exception>
#include <stdexcept>
 
#include <memory>
#include <iostream>

namespace CodeProject {

template<class TSrcType>
class TypeID : public std::unary_function<TSrcType,TSrcType>
{
public:
     typedef TSrcType     objTypeId;
};
 
template<class TKeyType,class TBaseType,class TParameterType>
class TObjFactory
{
public:
     typedef TBaseType *          value_type;
 
     TObjFactory(void) {}
     ~TObjFactory(void)
     {
          typeMapKeyToBuilder::iterator it(m_mapKeyToBuilder.begin()),itend(m_mapKeyToBuilder.end());
          for(;it != itend; ++it)
               delete it->second;
     }
 
     template<typename TSubType>
          void     registerBuilder(const TKeyType & key,TypeID<TSubType> obj)
          {
               typedef TypeID<TSubType>::objTypeId     srcType;
               typeMapKeyToBuilder::iterator it = m_mapKeyToBuilder.find(key);
               if (it != m_mapKeyToBuilder.end())
                    throw std::runtime_error("duplicate");
               m_mapKeyToBuilder[key] = new TObjBuilder<srcType>();
          }
 
     value_type     buildObj(const TKeyType & key, const TParameterType & param)
     {
          typeMapKeyToBuilder::iterator it = m_mapKeyToBuilder.find(key);
          if (it == m_mapKeyToBuilder.end())
               throw std::runtime_error("not found");
          return it->second->buildObject(param);
     }
protected:
     class TObjBuilderBase
     {
     public:
          virtual value_type     buildObject(const TParameterType & param) = 0;
     };
 
     template<class TSubType>
     class TObjBuilder : public TObjBuilderBase
     {
     public:
          virtual value_type     buildObject(const TParameterType & param)
               {     return new TSubType(param);     }
     };
 
     typedef TObjBuilderBase *                         typeBuilderPtr;
     typedef std::map<TKeyType,typeBuilderPtr>     typeMapKeyToBuilder;
     typeMapKeyToBuilder          m_mapKeyToBuilder;
};

}