﻿// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

namespace SharpDirect3D12
{
    /// <summary>
    /// Describes descriptors inline in the root signature version 1.1 that appear in shaders.
    /// </summary>
    public partial struct RootDescriptor1
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="RootDescriptor1"/> struct.
        /// </summary>
        /// <param name="shaderRegister">The shader register.</param>
        /// <param name="registerSpace">The register space.</param>
        /// <param name="flags">Specifies the <see cref="RootDescriptorFlags"/> that determine the volatility of descriptors and the data they reference.</param>
        public RootDescriptor1(int shaderRegister, int registerSpace, RootDescriptorFlags flags = RootDescriptorFlags.None)
        {
            ShaderRegister = shaderRegister;
            RegisterSpace = registerSpace;
            Flags = flags;
        }
    }
}
