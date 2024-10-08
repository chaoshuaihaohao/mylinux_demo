布谷鸟哈希怎么避免无限循环踢


布谷鸟哈希（Cuckoo Hashing）是一种用于解决哈希表冲突的算法，它通过维护两个哈希表（或数组）来允许同一个键在两个不同的位置。当插入一个新键时，如果发生冲突，算法会尝试将冲突的键“踢出”到另一个哈希表中。如果新键在另一个哈希表中也发生冲突，就会触发“踢出”过程，这个过程可能会无限进行下去，直到找到一个空位。

为了避免无限循环踢，布谷鸟哈希算法通常采用以下策略：

1. **随机选择踢出位置**：当需要踢出一个键时，随机选择一个位置进行踢出，而不是总是选择同一个位置。这有助于避免循环踢。

2. **限制踢出次数**：设置一个最大踢出次数的限制。如果达到最大次数仍然无法插入新键，则停止踢出过程，并可能选择重新哈希（rehashing）整个哈希表。

3. **重新哈希**：当踢出次数过多时，可能是哈希表负载过高，这时可以选择重新哈希。重新哈希意味着为所有键重新计算哈希值，并在新的哈希表中插入键。这通常涉及到创建两个新的哈希表，并逐步将旧表中的键迁移到新表中。

4. **动态调整负载因子**：保持哈希表的负载因子在一个合理的范围内，避免哈希表过于拥挤。当负载因子超过某个阈值时，可以通过增加哈希表的大小来减少冲突概率。

5. **使用两个独立的哈希函数**：布谷鸟哈希算法使用两个独立的哈希函数，这样可以减少键在两个哈希表中都发生冲突的概率。

6. **记录踢出路径**：在踢出过程中记录每个键的踢出路径，如果发现回到了起始点，说明可能发生了循环踢，这时可以停止踢出并采取其他措施。

7. **优化哈希函数**：选择或设计好的哈希函数，以减少冲突的概率。

在实际应用中，通常结合多种策略来避免无限循环踢，确保布谷鸟哈希算法的效率和稳定性。

本代码采用的是记录踢出路径的方式，如果当前踢出的key和之前的key相同，则认为踢出操作进入了循环。
