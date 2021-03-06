# Copyright (C) 2010 Google Inc. All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
# 
#     * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above
# copyright notice, this list of conditions and the following disclaimer
# in the documentation and/or other materials provided with the
# distribution.
#     * Neither the name of Google Inc. nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

from webkitpy.comments import bug_comment_from_commit_text
from webkitpy.steps.build import Build
from webkitpy.steps.commit import Commit
from webkitpy.steps.metastep import MetaStep
from webkitpy.steps.options import Options
from webkitpy.webkit_logging import log


class CompleteRollout(MetaStep):
    substeps = [
        Build,
        Commit,
    ]

    @classmethod
    def options(cls):
        collected_options = cls._collect_options_from_steps(cls.substeps)
        collected_options.append(Options.complete_rollout)
        return collected_options

    def run(self, state):
        bug_id = state["bug_id"]
        # FIXME: Fully automated rollout is not 100% idiot-proof yet, so for now just log with instructions on how to complete the rollout.
        # Once we trust rollout we will remove this option.
        if not self._options.complete_rollout:
            log("\nNOTE: Rollout support is experimental.\nPlease verify the rollout diff and use \"webkit-patch land %s\" to commit the rollout." % bug_id)
            return

        MetaStep.run(self, state)

        commit_comment = bug_comment_from_commit_text(self._tool.scm(), state["commit_text"])
        comment_text = "Reverted r%s for reason:\n\n%s\n\n%s" % (state["revision"], state["reason"], commit_comment)

        if not bug_id:
            log(comment_text)
            log("No bugs were updated.")
            return
        self._tool.bugs.reopen_bug(bug_id, comment_text)
