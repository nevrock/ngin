#include <snorri/animation.h>
#include <snorri/model_animation.h>

void Animation::readMissingBones(const aiAnimation* animation, ModelAnimation& model)
{
    int size = animation->mNumChannels;

    auto& boneInfoMap = model.getBoneInfoMap();//getting m_BoneInfoMap from ModelAnimation class
    int& boneCount = model.getBoneCount(); //getting the m_BoneCounter from ModelAnimation class

    //reading channels(bones engaged in an animation and their keyframes)
    for (int i = 0; i < size; i++)
    {
        auto channel = animation->mChannels[i];
        std::string boneName = channel->mNodeName.data;

        if (boneInfoMap.find(boneName) == boneInfoMap.end())
        {
            boneInfoMap[boneName].id = boneCount;
            boneCount++;
        }
        m_Bones.push_back(Bone(channel->mNodeName.data,
            boneInfoMap[channel->mNodeName.data].id, channel));
    }

    m_BoneInfoMap = boneInfoMap;
}
